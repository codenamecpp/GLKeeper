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

    // rebuild only invalidated tiles terrain mesh
    void UpdateTerrainMesh();

    // force rebuild all terrain mesh
    void BuildFullTerrainMesh();

    // tile mesh is invalidated and will be regenerated
    void HandleTileMeshInvalidated(MapTile* mapTile);

private:
    void CreateTerrainMeshList();
    void DestroyTerrainMeshList();
    
    void CreateWaterLavaMeshList();
    void DestroyWaterLavaMeshList();

private:
    std::vector<WaterLavaMesh*> mWaterLavaMeshArray;
    std::vector<TerrainMesh*> mTerrainMeshArray;

    TilesArray mInvalidatedTiles;
};

extern TerrainManager gTerrainManager;