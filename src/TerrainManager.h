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
    // setup terrain manager internal resources
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

    Entity* CreateTerrainMesh(const Rectangle& mapArea);
    Entity* CreateLavaMesh(const TilesList& tilesArray);
    Entity* CreateWaterMesh(const TilesList& tilesArray);

    Entity* GetObjectTerrainFromTile(const Point& tileLocation) const;

private:
    std::vector<Entity*> mWaterLavaMeshArray;
    std::vector<Entity*> mTerrainMeshArray;

    Entity* mHeightFieldDebugMesh = nullptr;

    TilesList mMeshInvalidatedTiles;
    TilesList mHighlightTiles;

    Texture2D_Image mHighlightTilesImage;
    Texture2D* mHighlightTilesTexture = nullptr;
};

extern TerrainManager gTerrainManager;