#pragma once

#include "Shaders.h"

// water and lava visualization manager
class WaterLavaMeshComponentRenderer: public cxx::noncopyable
{
    friend class WaterLavaMeshComponent;

public:
    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render water lava mesh for current render pass
    // @param renderContext: Current render context
    // @param component: Render component
    void Render(SceneRenderContext& renderContext, WaterLavaMeshComponent* component);

private:
    // setup renderable component mesh renderdata
    void PrepareRenderdata(WaterLavaMeshComponent* component);
    void ReleaseRenderdata(WaterLavaMeshComponent* component);

private:
    WaterLavaRenderProgram mWaterLavaRenderProgram;
};