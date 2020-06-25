#pragma once

#include "Shaders.h"

// water and lava visualization manager
class WaterLavaMeshRenderer: public cxx::noncopyable
{
    friend class RenderableWaterLavaMesh;

public:
    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render water lava mesh for current render pass
    // @param renderContext: Current render context
    // @param component: Render component
    void Render(SceneRenderContext& renderContext, RenderableWaterLavaMesh* component);

private:
    // setup renderable component mesh renderdata
    void PrepareRenderdata(RenderableWaterLavaMesh* component);
    void ReleaseRenderdata(RenderableWaterLavaMesh* component);

private:
    WaterLavaRenderProgram mWaterLavaRenderProgram;
};