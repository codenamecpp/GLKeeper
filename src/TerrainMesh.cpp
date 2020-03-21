#include "pch.h"
#include "TerrainMesh.h"
#include "SceneRenderList.h"
#include "GraphicsDevice.h"
#include "MapTile.h"
#include "GameWorld.h"
#include "GpuBuffer.h"

// limits
const int MaxTerrainMeshBufferSize = 1024 * 1024 * 2;

//////////////////////////////////////////////////////////////////////////

struct PieceBucket
{
public:
    std::deque<const TileMesh*> mTileMeshArray;

    int mVertexCount = 0;
    int mTriangleCount = 0;
};

typedef std::map<RenderMaterial, PieceBucket> PieceBucketMap;
struct PieceBucketContainer
{
public:
    PieceBucketMap mPieceBucketMap;

    int mVertexCount = 0;
    int mTrianglesCount = 0;
};

// Process mesh
template<typename TMeshPiecesContainer>
inline void SplitMeshPieces(const TMeshPiecesContainer& meshPieces, PieceBucketContainer& bucketContainer)
{
    for (const TileMesh& currTileMesh: meshPieces)
    {
        const int TriangleCount = (int) currTileMesh.mTriangles.size();
        const int VertexCount = (int) currTileMesh.mVertices.size();
        debug_assert(currTileMesh.mMaterial.mDiffuseTexture);
        if (!currTileMesh.mMaterial.mDiffuseTexture)
        {
            continue;
        }
        // allocate new bucket or request by material
        PieceBucket& pieceBucket = bucketContainer.mPieceBucketMap[currTileMesh.mMaterial];
        pieceBucket.mVertexCount += VertexCount;
        pieceBucket.mTriangleCount += TriangleCount;
        bucketContainer.mVertexCount += VertexCount;
        bucketContainer.mTrianglesCount += TriangleCount;
        pieceBucket.mTileMeshArray.emplace_back(&currTileMesh);
    }
}

//////////////////////////////////////////////////////////////////////////

TerrainMesh::TerrainMesh()
    : mVerticesBuffer()
    , mIndicesBuffer()
    , mMeshDirty()
{
}

TerrainMesh::~TerrainMesh()
{
    DestroyRenderData();
}

void TerrainMesh::SetTerrainArea(const Rect2D& mapArea)
{
    if (mMapTerrainRect == mapArea)
        return;

    mMapTerrainRect = mapArea;

    InvalidateMesh();

    cxx::aabbox sectorBox;
    // min
    sectorBox.mMin.x = (mMapTerrainRect.mX * DUNGEON_CELL_SIZE) - DUNGEON_CELL_HALF_SIZE;
    sectorBox.mMin.y = 0.0f;
    sectorBox.mMin.z = (mMapTerrainRect.mY * DUNGEON_CELL_SIZE) - DUNGEON_CELL_HALF_SIZE;
    // max
    sectorBox.mMax.x = sectorBox.mMin.x + (mMapTerrainRect.mSizeX * DUNGEON_CELL_SIZE);
    sectorBox.mMax.y = 3.0f;
    sectorBox.mMax.z = sectorBox.mMin.z + (mMapTerrainRect.mSizeY * DUNGEON_CELL_SIZE);

    SetLocalBoundingBox(sectorBox);
}

void TerrainMesh::InvalidateMesh()
{
    mMeshDirty = true;
}

void TerrainMesh::UpdateMesh()
{
    if (!mMeshDirty)
        return;

    PrepareRenderData();

    mMeshDirty = false;
}

void TerrainMesh::RegisterForRendering(SceneRenderList& renderList)
{
    // terrain is always opaque

    renderList.RegisterObject(eRenderPass_Opaque, this);
}

bool TerrainMesh::IsMeshInvalidated() const
{
    return mMeshDirty;
}

void TerrainMesh::DestroyRenderData()
{
    mBatchArray.clear();

    if (mVerticesBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mVerticesBuffer);
        mVerticesBuffer = nullptr;
    }

    if (mIndicesBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mIndicesBuffer);
        mIndicesBuffer = nullptr;
    }

    mMeshDirty = false;
}

void TerrainMesh::PrepareRenderData()
{
    if (mMapTerrainRect.mSizeX < 1 || mMapTerrainRect.mSizeY < 1)
    {
        debug_assert(false);
        return;
    }

    mBatchArray.clear();

    GameMap& gamemap = gGameWorld.mMapData;
    
    // prepare data for batching
    PieceBucketContainer pieceBucketContainer;

    for (int tiley = mMapTerrainRect.mY; tiley < (mMapTerrainRect.mY + mMapTerrainRect.mSizeY); ++tiley)
    for (int tilex = mMapTerrainRect.mX; tilex < (mMapTerrainRect.mX + mMapTerrainRect.mSizeX); ++tilex)
    {
        const Point2D tileLocation (tilex, tiley);
        if (!gamemap.IsWithinMap(tileLocation))
        {
            debug_assert(false);
            continue;
        }

        const MapTile* currTile = gamemap.GetMapTile(tileLocation);
        debug_assert(currTile);
        for (const TileFaceData& tileFace: currTile->mFaces)
        {
            SplitMeshPieces(tileFace.mMeshArray, pieceBucketContainer);
        }
    }

    // allocate buffers

    int actualVBufferLength = pieceBucketContainer.mVertexCount * Sizeof_Vertex3D_Terrain;
    int actualIBufferLength = pieceBucketContainer.mTrianglesCount * sizeof(glm::ivec3);

    if (actualVBufferLength == 0 || actualIBufferLength == 0)
    {
        debug_assert(false);
        return;
    }

    if (actualVBufferLength > MaxTerrainMeshBufferSize || actualIBufferLength > MaxTerrainMeshBufferSize)
    {
        debug_assert(false);
        return;
    }

    // allocate vertex buffer object
    if (mVerticesBuffer == nullptr)
    {
        mVerticesBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices);
        if (mVerticesBuffer == nullptr)
        {
            debug_assert(false);
            return;
        }
    }

    // allocate index buffer object
    if (mIndicesBuffer == nullptr)
    {
        mIndicesBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Indices);
        if (mIndicesBuffer == nullptr)
        {
            debug_assert(false);
            return;
        }
    }

    // setup buffers
    if (!mVerticesBuffer->Setup(eBufferUsage_Static, actualVBufferLength, nullptr) ||
        !mIndicesBuffer->Setup(eBufferUsage_Static, actualIBufferLength, nullptr))
    {
        debug_assert(false);
        return;
    }


    mBatchArray.resize(pieceBucketContainer.mPieceBucketMap.size());

    // compile geometries
    Vertex3D_Terrain* vbufferPtr = mVerticesBuffer->LockData<Vertex3D_Terrain>(BufferAccess_UnsynchronizedWrite, 0, actualVBufferLength);
    debug_assert(vbufferPtr);

    glm::ivec3* ibufferPtr = mIndicesBuffer->LockData<glm::ivec3>(BufferAccess_UnsynchronizedWrite, 0, actualIBufferLength);
    debug_assert(ibufferPtr);

    unsigned int imaterial = 0;
    unsigned int startVertex = 0;
    unsigned int startTriangle = 0;

    for (const auto& ebucket : pieceBucketContainer.mPieceBucketMap)
    {
        MeshBatch& meshBatch = mBatchArray[imaterial];
        meshBatch.mMaterial = ebucket.first;
        meshBatch.mTriangleCount = ebucket.second.mTriangleCount;
        meshBatch.mVertexCount = ebucket.second.mVertexCount;
        meshBatch.mTriangleStart = startTriangle;
        meshBatch.mVertexStart = startVertex;

        // copy geometry data
        unsigned int vertexoffset = 0;
        for (const TileMesh* currTileMesh: ebucket.second.mTileMeshArray)
        {
            // copy vertices
            const unsigned int groupNumVertices = currTileMesh->mVertices.size();
            ::memcpy(vbufferPtr, currTileMesh->mVertices.data(), groupNumVertices * Sizeof_Vertex3D_Terrain);
            // copy triangles
            const unsigned int groupNumTriangles = currTileMesh->mTriangles.size();
            for (unsigned int itriangle = 0; itriangle < groupNumTriangles; ++itriangle)
            {
                const glm::ivec3& srcTriangle = currTileMesh->mTriangles[itriangle];               
                ibufferPtr[itriangle].x = srcTriangle.x + vertexoffset + startVertex;
                ibufferPtr[itriangle].y = srcTriangle.y + vertexoffset + startVertex;
                ibufferPtr[itriangle].z = srcTriangle.z + vertexoffset + startVertex;
            }
            vertexoffset += groupNumVertices;
            ibufferPtr += groupNumTriangles;
            vbufferPtr += groupNumVertices;
        }

        startTriangle += meshBatch.mTriangleCount;
        startVertex += meshBatch.mVertexCount;
        ++imaterial;
    }

    if (!mIndicesBuffer->Unlock())
    {
        debug_assert(false);
    }

    if (!mVerticesBuffer->Unlock())
    {
        debug_assert(false);
    }
}