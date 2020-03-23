#pragma once

#include "RenderDefs.h"
#include "SceneDefs.h"
#include "WaterLavaRenderProgram.h"

// water and lava visualization manager
class WaterLavaMeshRenderer: public cxx::noncopyable
{
public:

    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render water lava mesh for current render pass
    // @param renderContext: Current render context
    // @param terrainMesh: Mesh instance
    void RenderWaterLavaMesh(SceneRenderContext& renderContext, WaterLavaMesh* waterLavaMesh);

private:
    WaterLavaRenderProgram mWaterLavaRenderProgram;
};