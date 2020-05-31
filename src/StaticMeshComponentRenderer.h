#pragma once

#include "Shaders.h"

class StaticMeshComponentRenderer: public cxx::noncopyable
{
    friend class StaticMeshComponent;

public:

    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render procedural mesh for current render pass
    // @param renderContext: Current render context
    // @param component: Renderable component
    void Render(SceneRenderContext& renderContext, StaticMeshComponent* component);

private:
    // setup renderable component mesh renderdata
    void PrepareRenderdata(StaticMeshComponent* component);
    void ReleaseRenderdata(StaticMeshComponent* component);

private:
    StaticMeshRenderProgram mStaticMeshRenderProgram;
};