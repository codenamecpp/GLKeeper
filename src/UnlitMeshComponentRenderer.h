#pragma once

#include "Shaders.h"

class UnlitMeshComponentRenderer: public cxx::noncopyable
{
    friend class UnlitMeshComponent;

public:

    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render unlit procedural mesh for current render pass
    // @param renderContext: Current render context
    // @param component: Renderable component
    void Render(SceneRenderContext& renderContext, UnlitMeshComponent* component);

private:
    // setup renderable component mesh renderdata
    void PrepareRenderdata(UnlitMeshComponent* component);
    void ReleaseRenderdata(UnlitMeshComponent* component);

private:
    UnlitMeshRenderProgram mUnlitRenderProgram;
};