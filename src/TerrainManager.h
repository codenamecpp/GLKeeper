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

    // rebuild terrain mesh for invalidated tiles
    void UpdateTerrainMesh();

private:
    void CreateTerrainMeshList();
    void DestroyTerrainMeshList();
    void BuildFullTerrainMesh();

    void CreateWaterLavaMeshList();
    void DestroyWaterLavaMeshList();

private:
    std::vector<WaterLavaMesh*> mWaterLavaMeshArray;
    std::vector<TerrainMesh*> mTerrainMeshArray;

    TilesArray mDirtyTilesArray;
};

extern TerrainManager gTerrainManager;