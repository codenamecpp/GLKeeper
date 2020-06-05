#pragma once

#include "Texture2D_Image.h"

// dungeon terrain manager
class TerrainManager: public cxx::noncopyable
{
    friend class TerrainMeshRenderer;

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

    GameObject* CreateObjectTerrain(const Rectangle& mapArea);
    GameObject* CreateObjectLava(const TilesList& tilesArray);
    GameObject* CreateObjectWater(const TilesList& tilesArray);

    GameObject* GetObjectTerrainFromTile(const Point& tileLocation) const;

private:
    std::vector<GameObject*> mWaterLavaMeshArray;
    std::vector<GameObject*> mTerrainMeshArray;

    TilesList mMeshInvalidatedTiles;
    TilesList mHighlightTiles;

    Texture2D_Image mHighlightTilesImage;
    Texture2D* mHighlightTilesTexture = nullptr;
};

extern TerrainManager gTerrainManager;