#pragma once

#include "GameDefs.h"
#include "MapTile.h"
#include "ShaderProgram.h"
#include "SceneDefs.h"
#include "GameSessionAware.h"

class Camera;

//////////////////////////////////////////////////////////////////////////
// Dungeon Render Mesh 
//////////////////////////////////////////////////////////////////////////

class TerrainRenderer final: private GameSessionAware
{
public:
    TerrainRenderer();

    // First time rendered initialization
    // All shaders, buffers and other graphics resources might be loaded here
    // Return false on error
    bool Initialize();
    void Shutdown();

    // Generate whole dungeon mesh geometry
    void CreateTerrainMesh();

    // Reset dungeon mesh geometry
    void CleanupTerrainMesh();

    // Render mesh
    void Render(Camera& camera);

    // Notify render that tile mesh was modified and should be upload to video card
    void InvalidateTile(const MapPoint2D& theTileLocation);
    void InvalidateTile(const MapTile* theTile)
    {
        InvalidateTile(theTile->mTileLocation);
    }

    void TileHighlightChanged(MapTile* mapTile);

private:
    // Processing geometries
    bool BuildSector(int theSectorX, int theSectorY);

    // Force update highhlight tiles texture
    void CommitHighlightTiles();

private:

    // Internals
    struct SectorBatch
    {
        SurfaceMaterial mMaterial;
        unsigned int mVertexStart;
        unsigned int mVertexCount;
        unsigned int mTriangleStart;
        unsigned int mTriangleCount;
    };

    using SectorBatchArray = std::vector<SectorBatch>;
    struct Sector
    {
        std::unique_ptr<GpuVertexBuffer> mVertexBuffer; // vertex buffer
        std::unique_ptr<GpuIndexBuffer> mIndexBuffer; // index buffer
        SectorBatchArray mSectorBatches;
        bool mDirty;
    };

    ShaderProgram_Terrain* mShaderProgram = nullptr;

    // tagged tiles
    std::unique_ptr<GpuTexture2D> mHighlightTilesTexture;
    BitmapImage mHighlightTilesBitmap;

    std::vector<MapTile*> mHighlightTilesChanged;

    int mLevelSizeX; // blocks per width
    int mLevelSizeY; // blocks per height
    int mSectorsX; // sectors per width
    int mSectorsY; // sectors per height
    std::vector<Sector> mSectorArray;

    bool mSectorCulling; // render flags
};