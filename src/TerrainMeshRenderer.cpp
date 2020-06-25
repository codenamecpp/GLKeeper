#include "pch.h"
#include "TerrainMeshRenderer.h"
#include "RenderScene.h"
#include "GraphicsDevice.h"
#include "RenderableTerrainMesh.h"
#include "cvars.h"
#include "TerrainTile.h"
#include "GameWorld.h"
#include "GpuBuffer.h"
#include "TerrainManager.h"
#include "Texture2D.h"

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

typedef std::map<MeshMaterial, PieceBucket> PieceBucketMap;
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

bool TerrainMeshRenderer::Initialize()
{
    if (!mTerrainRenderProgram.LoadProgram())
    {
        debug_assert(false);
    }
    return true;
}

void TerrainMeshRenderer::Deinit()
{
    mTerrainRenderProgram.FreeProgram();
}

void TerrainMeshRenderer::Render(SceneRenderContext& renderContext, RenderableTerrainMesh* component)
{
    if (!gCVarRender_DrawTerrain.mValue)
        return;

    debug_assert(component);
    if (component->mDrawParts.empty())
    {
        debug_assert(false);
        return;
    }

    mTerrainRenderProgram.SetViewProjectionMatrix(gRenderScene.mCamera.mViewProjectionMatrix);
    mTerrainRenderProgram.SetModelMatrix(SceneIdentyMatrix);
    mTerrainRenderProgram.ActivateProgram();

    // bind additional highlight tiles texture
    if (gTerrainManager.mHighlightTilesTexture)
    {
        gTerrainManager.mHighlightTilesTexture->ActivateTexture(eTextureUnit_1);
    }

    // bind indices
    gGraphicsDevice.BindIndexBuffer(component->mIndexBuffer);
    gGraphicsDevice.BindVertexBuffer(component->mVertexBuffer, Vertex3D_Terrain_Format::Get());

    for (RenderableTerrainMesh::DrawPart& currBatch: component->mDrawParts)
    {
        if (currBatch.mVertexCount == 0)
        {
            debug_assert(false);
            continue;
        }
        MeshMaterial* currMaterial = component->GetMeshMaterial(currBatch.mMaterialIndex);
        if (currMaterial == nullptr)
        {
            debug_assert(false);
            continue;
        }
        // filter out submeshes depending on current render pass
        if (renderContext.mCurrentPass == eRenderPass_Translucent && !currMaterial->IsTransparent())
            continue;

        if (renderContext.mCurrentPass == eRenderPass_Opaque && currMaterial->IsTransparent())
            continue;

        currMaterial->ActivateMaterial();
        gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32,
            currBatch.mIndexDataOffset, 
            currBatch.mTriangleCount * 3);
    }
}

void TerrainMeshRenderer::ReleaseRenderdata(RenderableTerrainMesh* component)
{
    debug_assert(component);
    component->mRenderProgram = nullptr;
    if (component->mVertexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(component->mVertexBuffer);
        component->mVertexBuffer = nullptr;
    }

    if (component->mIndexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(component->mIndexBuffer);
        component->mIndexBuffer = nullptr;
    }
    component->InvalidateMesh();
    component->ClearMeshMaterials();
}

void TerrainMeshRenderer::PrepareRenderdata(RenderableTerrainMesh* component)
{
    debug_assert(component);

    const Rectangle& rcMapTerrain = component->mMapTerrainRect;
    if (rcMapTerrain.w < 1 || rcMapTerrain.h < 1)
    {
        debug_assert(false);
        return;
    }

    component->ClearMeshMaterials();
    component->ClearDrawParts();

    GameMap& gamemap = gGameWorld.mMapData;
    
    // prepare data for batching
    PieceBucketContainer pieceBucketContainer;

    for (int tiley = rcMapTerrain.y; tiley < (rcMapTerrain.y + rcMapTerrain.h); ++tiley)
    for (int tilex = rcMapTerrain.x; tilex < (rcMapTerrain.x + rcMapTerrain.w); ++tilex)
    {
        const Point tileLocation (tilex, tiley);
        const TerrainTile* currTile = gamemap.GetMapTile(tileLocation);

        if (currTile == nullptr)
        {
            debug_assert(false);
            continue;
        }

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
    if (component->mVertexBuffer == nullptr)
    {
        component->mVertexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices);
        if (component->mVertexBuffer == nullptr)
        {
            debug_assert(false);
            return;
        }
    }

    // allocate index buffer object
    if (component->mIndexBuffer == nullptr)
    {
        component->mIndexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Indices);
        if (component->mIndexBuffer == nullptr)
        {
            debug_assert(false);
            return;
        }
    }

    GpuBuffer* vertexBuffer = component->mVertexBuffer;
    GpuBuffer* indexBuffer = component->mIndexBuffer;

    // setup buffers
    if (!vertexBuffer->Setup(eBufferUsage_Static, actualVBufferLength, nullptr) ||
        !indexBuffer->Setup(eBufferUsage_Static, actualIBufferLength, nullptr))
    {
        debug_assert(false);
        return;
    }

    int numPieces = (int) pieceBucketContainer.mPieceBucketMap.size();
    debug_assert(numPieces > 0);
    component->SetDrawPartsCount(numPieces);
    component->SetMeshMaterialsCount(numPieces);

    // compile geometries
    Vertex3D_Terrain* vbufferPtr = vertexBuffer->LockData<Vertex3D_Terrain>(BufferAccess_UnsynchronizedWrite, 0, actualVBufferLength);
    debug_assert(vbufferPtr);

    glm::ivec3* ibufferPtr = indexBuffer->LockData<glm::ivec3>(BufferAccess_UnsynchronizedWrite, 0, actualIBufferLength);
    debug_assert(ibufferPtr);

    unsigned int imaterial = 0;
    unsigned int startVertex = 0;
    unsigned int startTriangle = 0;

    for (const auto& ebucket : pieceBucketContainer.mPieceBucketMap)
    {
        component->SetMeshMaterial(imaterial, ebucket.first);

        RenderableTerrainMesh::DrawPart& drawCall = component->mDrawParts[imaterial];
        drawCall.mMaterialIndex = imaterial;
        drawCall.mTriangleCount = ebucket.second.mTriangleCount;
        drawCall.mVertexCount = ebucket.second.mVertexCount;
        drawCall.mIndexDataOffset = startTriangle * sizeof(glm::ivec3);
        drawCall.mVertexDataOffset = startVertex * Sizeof_Vertex3D_Terrain;

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

        startTriangle += drawCall.mTriangleCount;
        startVertex += drawCall.mVertexCount;
        ++imaterial;
    }

    if (!indexBuffer->Unlock())
    {
        debug_assert(false);
    }

    if (!vertexBuffer->Unlock())
    {
        debug_assert(false);
    }

    component->mRenderProgram = &mTerrainRenderProgram;
}