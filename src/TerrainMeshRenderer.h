#pragma once

#include "ResourceDefs.h"
#include "RenderDefs.h"
#include "TerrainRenderProgram.h"
#include "SceneDefs.h"

// terrain visualization manager
class TerrainMeshRenderer: public cxx::noncopyable
{
public:

    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render terrain mesh for current render pass
    // @param renderContext: Current render context
    // @param terrainMesh: Mesh instance
    void RenderTerrainMesh(SceneRenderContext& renderContext, TerrainMesh* terrainMesh);

private:
    TerrainRenderProgram mTerrainRenderProgram;
};