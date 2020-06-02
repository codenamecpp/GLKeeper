#pragma once

// dungeon terrain manager
class TerrainManager: public cxx::noncopyable
{
public:
    // setup terrain manager internal resources
    bool Initialize();
    void Deinit();

    void EnterWorld();
    void ClearWorld();

    // rebuild only invalidated tiles or full terrain mesh
    void UpdateTerrainMesh();
    void BuildFullTerrainMesh();

    // tile mesh is invalidated and will be regenerated
    void InvalidateTileMesh(MapTile* mapTile);

    void ClearInvalidatedTiles();

private:
    void CreateTerrainMeshList();
    void DestroyTerrainMeshList();
    
    void CreateWaterLavaMeshList();
    void DestroyWaterLavaMeshList();

    GameObject* CreateObjectTerrain(const Rectangle& mapArea);
    GameObject* CreateObjectLava(const TilesArray& tilesArray);
    GameObject* CreateObjectWater(const TilesArray& tilesArray);

private:
    std::vector<GameObject*> mWaterLavaMeshArray;
    std::vector<GameObject*> mTerrainMeshArray;

    TilesArray mInvalidatedTiles;
};

extern TerrainManager gTerrainManager;