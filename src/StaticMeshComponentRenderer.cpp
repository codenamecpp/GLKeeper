#include "pch.h"
#include "StaticMeshComponentRenderer.h"
#include "GpuBuffer.h"

bool StaticMeshComponentRenderer::Initialize()
{
    if (!mStaticMeshRenderProgram.LoadProgram())
    {
        debug_assert(false);
    }

    return true;
}

void StaticMeshComponentRenderer::Deinit()
{
    mStaticMeshRenderProgram.FreeProgram();
}

void StaticMeshComponentRenderer::Render(SceneRenderContext& renderContext, StaticMeshComponent* component)
{
}

void StaticMeshComponentRenderer::PrepareRenderdata(StaticMeshComponent* component)
{
}

void StaticMeshComponentRenderer::ReleaseRenderdata(StaticMeshComponent* component)
{
}