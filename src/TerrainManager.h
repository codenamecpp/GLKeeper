#pragma once

#include "GameDefs.h"
#include "SceneDefs.h"

// dungeon terrain manager
class TerrainManager: public cxx::noncopyable
{
public:
    
    // setup terrain manager internal resources
    bool Initialize();
    void Deinit();

    void EnterWorld();
    void ClearWorld();

    // rebuild only invalidated tiles terrain mesh
    void UpdateTerrainMesh();

    // force rebuild all terrain mesh
    void BuildFullTerrainMesh();

    // tile mesh is invalidated and will be regenerated
    void HandleTileMeshInvalidated(MapTile* mapTile);

    // reset invalidated flag for all queued tiles
    void ClearInvalidated();

private:
    void CreateTerrainMeshList();
    void DestroyTerrainMeshList();
    
    void CreateWaterLavaMeshList();
    void DestroyWaterLavaMeshList();

    GameObject* CreateObjectTerrain(const Rect2D& mapArea);
    GameObject* CreateObjectLava(const TilesArray& tilesArray);
    GameObject* CreateObjectWater(const TilesArray& tilesArray);

private:
    std::vector<GameObject*> mWaterLavaMeshArray;
    std::vector<GameObject*> mTerrainMeshArray;

    TilesArray mInvalidatedTiles;
};

extern TerrainManager gTerrainManager;