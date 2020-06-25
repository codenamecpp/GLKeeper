#pragma once

#include "Shaders.h"

// terrain visualization manager
class TerrainMeshRenderer: public cxx::noncopyable
{
    friend class RenderableTerrainMesh;

public:

    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render terrain mesh for current render pass
    // @param renderContext: Current render context
    // @param component: Renderable component
    void Render(SceneRenderContext& renderContext, RenderableTerrainMesh* component);

private:
    // setup renderable component mesh renderdata
    void PrepareRenderdata(RenderableTerrainMesh* component);
    void ReleaseRenderdata(RenderableTerrainMesh* component);

private:
    TerrainRenderProgram mTerrainRenderProgram;
};