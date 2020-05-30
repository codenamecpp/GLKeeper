#pragma once

#include "TerrainRenderProgram.h"

// terrain visualization manager
class TerrainMeshRenderer: public cxx::noncopyable
{
    friend class TerrainMeshComponent;

public:

    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render terrain mesh for current render pass
    // @param renderContext: Current render context
    // @param component: Renderable component
    void Render(SceneRenderContext& renderContext, TerrainMeshComponent* component);

private:
    // setup renderable component mesh renderdata
    void PrepareRenderdata(TerrainMeshComponent* component);
    void ReleaseRenderdata(TerrainMeshComponent* component);

private:
    TerrainRenderProgram mTerrainRenderProgram;
};