#pragma once

#include "TerrainRenderProgram.h"

// terrain visualization manager
class TerrainMeshRenderer: public cxx::noncopyable
{
public:

    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render terrain mesh for current render pass
    // @param renderContext: Current render context
    // @param component: Renderable component
    void Render(SceneRenderContext& renderContext, TerrainMeshComponent* component);

private:
    TerrainRenderProgram mTerrainRenderProgram;
};