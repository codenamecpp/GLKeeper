#include "stdafx.h"
#include "TerrainRenderer.h"
#include "GameMap.h"
#include "GameRenderManager.h"
#include "Camera.h"
#include "ShadersManager.h"
#include "GameWorld.h"
#include "GameMain.h"

//////////////////////////////////////////////////////////////////////////

// Limits
enum { SECTOR_SIZE = 8 }; // NxN blocks
enum { MAX_VBUFFER_LENGTH = 1024 * 1024 * 2 }; // max vertex buffer size in bytes
enum { MAX_IBUFFER_LENGTH = 1024 * 1024 * 2 }; // max index buffer size in bytes

// color constants
const Color32 TILE_TAGGED_COLOR = MAKE_RGBA(64, 64, 255, 0);

//////////////////////////////////////////////////////////////////////////

struct PieceBucketGeometry
{
    const TerrainVertex3D* mVertexDataPtr = nullptr;
    const glm::ivec3* mTriangleDataPtr = nullptr;
    unsigned int mVertexCount = 0;
    unsigned int mTriangleCount = 0;
};

struct PieceBucket
{
    std::vector<PieceBucketGeometry> mGeometries;
    unsigned int mVertexCount = 0;
    unsigned int mTriangleCount = 0;
};

typedef std::map<SurfaceMaterial, PieceBucket> PieceBucketMap;

struct PieceBucketContainer
{
    PieceBucketMap mPieceBucketMap;
    unsigned int mVertexCount = 0;
    unsigned int mTrianglesCount = 0;
};

// Process mesh
inline void SplitMeshPieces(const TileFaceMesh& faceMesh, PieceBucketContainer& theContainer)
{
    for (const TileFaceMesh::Piece& piece: faceMesh.mPieces)
    {
        cxx_assert(piece.mMaterial.mDiffuseTexture);
        if (piece.mMaterial.mDiffuseTexture == nullptr)
        {
            continue;
        }

        // allocate new bucket or request by material
        PieceBucket& pieceBucket = theContainer.mPieceBucketMap[piece.mMaterial];
        {
            pieceBucket.mVertexCount += piece.mVertexCount;
            pieceBucket.mTriangleCount += piece.mTriangleCount;
            // total geometry elemenets count
            theContainer.mVertexCount += piece.mVertexCount;
            theContainer.mTrianglesCount += piece.mTriangleCount;
        }

        PieceBucketGeometry& geo = pieceBucket.mGeometries.emplace_back();
        {
            geo.mVertexDataPtr = &faceMesh.mVertices[piece.mBaseVertex];
            geo.mVertexCount = piece.mVertexCount;
            geo.mTriangleDataPtr = &faceMesh.mTriangles[piece.mTrianglesOffset];
            geo.mTriangleCount = piece.mTriangleCount;
        }
    }
}

//////////////////////////////////////////////////////////////////////////

TerrainRenderer::TerrainRenderer()
    : mLevelSizeX()
    , mLevelSizeY()
    , mSectorsX()
    , mSectorsY()
    , mSectorCulling(true)
{
}

bool TerrainRenderer::Initialize()
{
    mShaderProgram = gShadersManager.GetProgramOfType<ShaderProgram_Terrain>("mesh_terrain");
    cxx_assert(mShaderProgram);
    if (mShaderProgram == nullptr)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load mesh_terrain shader program");
        return false;
    }

    mShaderProgram->InitRenderData();

    mHighlightTilesChanged.reserve(1024);
    return true;
}

void TerrainRenderer::Shutdown()
{
    mShaderProgram = nullptr;
    CleanupTerrainMesh();
}

void TerrainRenderer::Render(Camera& camera)
{
    CommitHighlightTiles();

    gRenderDevice.BindTexture2D(eTextureUnit_DiffuseMap1, mHighlightTilesTexture.get());

    // setup constants
    static glm::mat4 indentMatrix(1.0f);
    mShaderProgram->SetViewProjectionMatrix(camera.mViewProjectionMatrix);
    mShaderProgram->SetModelMatrix(indentMatrix);
    mShaderProgram->BindProgram();

    int sectorsDirty = 0;
    for (int iSectorY = 0; iSectorY < mSectorsY; ++iSectorY)
    for (int iSectorX = 0; iSectorX < mSectorsX; ++iSectorX)
    {
        Sector& theSector = mSectorArray[iSectorX + iSectorY * mSectorsX];

        // culling
        if (mSectorCulling)
        {
            cxx::aabbox sectorBox;
            // min
            sectorBox.mMin.x = ((iSectorX * MAP_TILE_SIZE) * (SECTOR_SIZE * MAP_TILE_SIZE)) - MAP_TILE_HALF_SIZE;
            sectorBox.mMin.y = 0.0f;
            sectorBox.mMin.z = ((iSectorY * MAP_TILE_SIZE) * (SECTOR_SIZE * MAP_TILE_SIZE)) - MAP_TILE_HALF_SIZE;
            // max
            sectorBox.mMax.x = sectorBox.mMin.x + (SECTOR_SIZE * MAP_TILE_SIZE);
            sectorBox.mMax.y = 3.0f;
            sectorBox.mMax.z = sectorBox.mMin.z + (SECTOR_SIZE * MAP_TILE_SIZE);

            if (!camera.mFrustum.contains(sectorBox))
                continue;
        }

        if (theSector.mDirty)
        {
            ++sectorsDirty;
            BuildSector(iSectorX, iSectorY);
        }

        if (!theSector.mVertexBuffer)
        {
            cxx_assert(!"vertex buffer is null");
            continue;
        }

        gRenderDevice.BindVertexBuffer(theSector.mVertexBuffer.get());
        gRenderDevice.BindIndexBuffer(theSector.mIndexBuffer.get());

        // process submeshes
        for (SectorBatch& sectorBatch: theSector.mSectorBatches)
        {
            sectorBatch.mMaterial.BindMaterial(*mShaderProgram);
            // render submesh
            gRenderDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32,
                sectorBatch.mTriangleStart * sizeof(glm::ivec3), 
                sectorBatch.mTriangleCount * 3);
        }
    } // for
}

void TerrainRenderer::InvalidateTile(const MapPoint2D& theTileLocation)
{
    int theSectorX = theTileLocation.x / SECTOR_SIZE;
    int theSectorY = theTileLocation.y / SECTOR_SIZE;

    cxx_assert(theSectorX < mSectorsX && theSectorX > -1);
    cxx_assert(theSectorY < mSectorsY && theSectorY > -1);
    
    Sector& theSector = mSectorArray[theSectorX + theSectorY * mSectorsX];
    theSector.mDirty = 1; // will be rebuild next frame
}

void TerrainRenderer::TileHighlightChanged(MapTile* mapTile)
{
    cxx_assert(mapTile);

    if (mapTile == nullptr) return;

    if (!cxx::contains(mHighlightTilesChanged, mapTile))
    {
        mHighlightTilesChanged.push_back(mapTile);
    }
}

void TerrainRenderer::CreateTerrainMesh()
{
    const MapPoint2D& mapDimensions = GetGameWorld().GetGameMap().GetDimensions();

    mLevelSizeX = mapDimensions.x;
    mLevelSizeY = mapDimensions.y;

    mSectorsX = (mLevelSizeX / SECTOR_SIZE) + ((mLevelSizeX % SECTOR_SIZE) ? 1 : 0);
    mSectorsY = (mLevelSizeY / SECTOR_SIZE) + ((mLevelSizeY % SECTOR_SIZE) ? 1 : 0);

    // allocate sectors
    mSectorArray.resize(mSectorsX * mSectorsY);

    for (int iSectorY = 0; iSectorY < mSectorsY; ++iSectorY)
    for (int iSectorX = 0; iSectorX < mSectorsX; ++iSectorX)
    {
        if (!BuildSector(iSectorX, iSectorY))
        {
            gConsole.LogMessage(eLogLevel_Error, "Failed to build dungeon mesh sector %d-%d", iSectorX, iSectorY);
        }
    }

    // prepare highlight tiles texture
    Point2D maxDims { MAX_DUNGEON_MAP_DIMENSIONS, MAX_DUNGEON_MAP_DIMENSIONS };
    if (!mHighlightTilesBitmap.Create(ePixelFormat_RGB8, maxDims, COLOR_BLACK))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot allocate tiles highlight texture (1)");
    }

    mHighlightTilesTexture = gRenderDevice.CreateTexture2D(mHighlightTilesBitmap);
    if (!mHighlightTilesTexture)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot allocate tiles highlight texture (2)");
    }
}

bool TerrainRenderer::BuildSector(int theSectorX, int theSectorY)
{
    cxx_assert(theSectorX < mSectorsX && theSectorX > -1);
    cxx_assert(theSectorY < mSectorsY && theSectorY > -1);

    Sector& theSector = mSectorArray[theSectorX + theSectorY * mSectorsX];

    theSector.mSectorBatches.clear();
    
    const GameMap& gameMap = GetGameWorld().GetGameMap();

    // prepare data for batching
    PieceBucketContainer pieceBucketContainer;

    for (int blockY = 0; blockY < SECTOR_SIZE; ++blockY)
    for (int blockX = 0; blockX < SECTOR_SIZE; ++blockX)
    {
        const MapPoint2D tileLocation {
            blockX + theSectorX * SECTOR_SIZE, 
            blockY + theSectorY * SECTOR_SIZE
        };

        if (!gameMap.WithinMap(tileLocation))
            continue;

        const MapTile* targetMapTile = gameMap.GetMapTile(tileLocation);
        // process tile geometry
        for (const TileFaceData& tileFace: targetMapTile->mFaces)
        {
            SplitMeshPieces(tileFace.mFaceMesh, pieceBucketContainer);
        }
    }

    // allocate buffers

    unsigned int actualVBufferLength = pieceBucketContainer.mVertexCount * Sizeof_TerrainVertex;
    unsigned int actualIBufferLength = pieceBucketContainer.mTrianglesCount * sizeof(glm::ivec3);

    if (actualVBufferLength == 0 || actualIBufferLength == 0)
        return true; // be tolerant

    if (actualVBufferLength > MAX_VBUFFER_LENGTH || actualIBufferLength > MAX_IBUFFER_LENGTH)
    {
        cxx_assert(!"buffer length is invalid");
        return false;
    }

    if (!theSector.mVertexBuffer)
    {
        theSector.mVertexBuffer = gRenderDevice.CreateVertexBuffer(eBufferUsage_Static, actualVBufferLength);
        if (!theSector.mVertexBuffer)
        {
            cxx_assert(!"cannot allocate buffer");
            return false;
        }

        theSector.mVertexBuffer->ConfigureVertexDefinitions(TerrainVertex3D_Format::Get());
    }
    else
    {
        theSector.mVertexBuffer->Create(eBufferUsage_Static, actualVBufferLength, nullptr);
    }

    if (!theSector.mIndexBuffer)
    {
        theSector.mIndexBuffer = gRenderDevice.CreateIndexBuffer(eBufferUsage_Static, actualIBufferLength);
        if (!theSector.mIndexBuffer)
        {
            cxx_assert(!"cannot allocate buffer");
            return false;
        }
    }
    else
    {
        theSector.mIndexBuffer->Create(eBufferUsage_Static, actualIBufferLength, nullptr);
    }

    theSector.mSectorBatches.resize(pieceBucketContainer.mPieceBucketMap.size());

    // compile geometries

    TerrainVertex3D* vbufferPtr = theSector.mVertexBuffer->LockData<TerrainVertex3D>(BufferAccess_Write, 0, actualVBufferLength);
    cxx_assert(vbufferPtr);

    glm::ivec3* ibufferPtr = theSector.mIndexBuffer->LockData<glm::ivec3>(BufferAccess_Write, 0, actualIBufferLength);
    cxx_assert(ibufferPtr);

    unsigned int imaterial = 0;
    unsigned int startVertex = 0;
    unsigned int startTriangle = 0;

    for (const auto& ebucket : pieceBucketContainer.mPieceBucketMap)
    {
        SectorBatch& sectorBatch = theSector.mSectorBatches[imaterial];
        sectorBatch.mMaterial = ebucket.first;
        sectorBatch.mTriangleCount = ebucket.second.mTriangleCount;
        sectorBatch.mVertexCount = ebucket.second.mVertexCount;
        sectorBatch.mTriangleStart = startTriangle;
        sectorBatch.mVertexStart = startVertex;

        // copy geometry data
        unsigned int vertexoffset = 0;
        for (const PieceBucketGeometry& geo: ebucket.second.mGeometries)
        {
            // copy vertices
            ::memcpy(vbufferPtr, geo.mVertexDataPtr, geo.mVertexCount * Sizeof_TerrainVertex);
            // copy triangles
            for (unsigned int itriangle = 0; itriangle < geo.mTriangleCount; ++itriangle)
            {
                const glm::ivec3& srcTriangle = geo.mTriangleDataPtr[itriangle];               
                ibufferPtr[itriangle].x = srcTriangle.x + vertexoffset + startVertex;
                ibufferPtr[itriangle].y = srcTriangle.y + vertexoffset + startVertex;
                ibufferPtr[itriangle].z = srcTriangle.z + vertexoffset + startVertex;
            }
            vertexoffset += geo.mVertexCount;
            ibufferPtr += geo.mTriangleCount;
            vbufferPtr += geo.mVertexCount;
        }

        startTriangle += sectorBatch.mTriangleCount;
        startVertex += sectorBatch.mVertexCount;
        ++imaterial;
    }

    theSector.mIndexBuffer->Unlock();
    theSector.mVertexBuffer->Unlock();
    theSector.mDirty = 0; // up to date
    return true;
}

void TerrainRenderer::CommitHighlightTiles()
{
    if (mHighlightTilesChanged.empty() || !mHighlightTilesTexture)
        return;

    unsigned char* pixels = mHighlightTilesBitmap.GetMipPixels(0);

    int highlightTilesTextureWidth = mHighlightTilesTexture->GetTextureWidth();
    for (MapTile* currentTile: mHighlightTilesChanged)
    {
        int offset = currentTile->mTileLocation.y * highlightTilesTextureWidth + currentTile->mTileLocation.x;
        if (currentTile->mIsTagged)
        {
            pixels[offset * 3 + 0] = TILE_TAGGED_COLOR.mR;
            pixels[offset * 3 + 1] = TILE_TAGGED_COLOR.mG;
            pixels[offset * 3 + 2] = TILE_TAGGED_COLOR.mB;
        }
        else
        {
            pixels[offset * 3 + 0] = 0;
            pixels[offset * 3 + 1] = 0;
            pixels[offset * 3 + 2] = 0;
        }
    }
    mHighlightTilesChanged.clear();

    // upload texture data
    mHighlightTilesTexture->Invalidate();
    mHighlightTilesTexture->Upload(pixels);
}

void TerrainRenderer::CleanupTerrainMesh()
{
    mLevelSizeX = 0;
    mLevelSizeY = 0;
    mSectorsX = 0;
    mSectorsY = 0;
    mSectorArray.clear();
    mHighlightTilesBitmap.Clear();
    mHighlightTilesTexture.reset();
    mHighlightTilesChanged.clear();
}
