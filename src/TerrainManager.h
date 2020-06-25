#pragma once

#include "Texture2D_Image.h"
#include "TerrainHeightField.h"

// dungeon terrain manager
class TerrainManager: public cxx::noncopyable
{
    friend class TerrainMeshRenderer;

public:
    // readonly
    TerrainHeightField mHeightField;

public:
    // one time initialization/shutdown routine
    bool Initialize();
    void Deinit();

    void EnterWorld();
    void ClearWorld();

    void PreRenderScene();

    // rebuild only invalidated tiles or full terrain mesh
    void UpdateTerrainMesh();
    void BuildFullTerrainMesh();

    // tile mesh is invalidated and will be regenerated
    void InvalidateTileMesh(TerrainTile* terrainTile);
    void InvalidateTileNeighboursMesh(TerrainTile* terrainTile);
    void ClearInvalidatedTiles();

    // enable or disable highhlight for tile
    void HighhlightTile(TerrainTile* terrainTile, bool isHighlighted);

private:
    void InitTerrainMeshList();
    void FreeTerrainMeshList();
    
    void InitWaterLavaMeshList();
    void FreeWaterLavaMeshList();

    void InitHighhlightTilesTexture();
    void FreeHighhlightTilesTexture();
    void UpdateHighhlightTilesTexture();

    void InitHeightFieldDebugMesh();
    void FreeHeightFieldDebugMesh();
    void UpdateHeightFieldDebugMesh();

    // create water or lava renderable object
    RenderableWaterLavaMesh* CreateLavaMesh(const TilesList& tilesArray);
    RenderableWaterLavaMesh* CreateWaterMesh(const TilesList& tilesArray);
    // create terrain renderable object
    RenderableTerrainMesh* CreateTerrainMesh(const Rectangle& mapArea);
    // get terrain renderable object from map coordinate
    RenderableTerrainMesh* GetObjectTerrainFromTile(const Point& tileLocation) const;

private:
    std::vector<RenderableWaterLavaMesh*> mWaterLavaMeshArray;
    std::vector<RenderableTerrainMesh*> mTerrainMeshArray;

    RenderableProcMesh* mHeightFieldDebugMesh = nullptr;

    TilesList mMeshInvalidatedTiles;
    TilesList mHighlightTiles;

    Texture2D_Image mHighlightTilesImage;
    Texture2D* mHighlightTilesTexture = nullptr;
};

extern TerrainManager gTerrainManager;