#pragma once

#include "SceneDefs.h"

// dungeon terrain manager
class TerrainManager: public cxx::noncopyable
{
public:
    
    // setup terrain manager internal resources
    bool Initialize();
    void Deinit();

private:
    void CreateTerrainMeshList();
    void DestroyTerrainMeshList();

private:
    std::vector<TerrainMesh*> mTerrainMeshArray;
};