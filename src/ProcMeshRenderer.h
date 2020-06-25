#pragma once

#include "Shaders.h"

class ProcMeshRenderer: public cxx::noncopyable
{
    friend class RenderableProcMesh;

public:
    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render procedural mesh for current render pass
    // @param renderContext: Current render context
    // @param component: Renderable component
    void Render(SceneRenderContext& renderContext, RenderableProcMesh* component);

private:
    // setup renderable component mesh renderdata
    void PrepareRenderdata(RenderableProcMesh* component);
    void ReleaseRenderdata(RenderableProcMesh* component);

private:
    ProcMeshRenderProgram mProcMeshRenderProgram;
};