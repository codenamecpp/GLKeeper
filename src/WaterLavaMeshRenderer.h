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
    // @param component: Render component
    void Render(SceneRenderContext& renderContext, WaterLavaMeshComponent* component);

private:
    WaterLavaRenderProgram mWaterLavaRenderProgram;
};