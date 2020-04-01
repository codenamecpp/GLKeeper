#pragma once

#include "RenderDefs.h"
#include "SceneDefs.h"
#include "WaterLavaRenderProgram.h"
#include "GameDefs.h"

// water and lava visualization manager
class WaterLavaMeshRenderer: public cxx::noncopyable
{
public:

    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render water lava mesh for current render pass
    // @param renderContext: Current render context
    // @param component: Render component
    void Render(SceneRenderContext& renderContext, WaterLavaMesh* component);

private:
    WaterLavaRenderProgram mWaterLavaRenderProgram;
};