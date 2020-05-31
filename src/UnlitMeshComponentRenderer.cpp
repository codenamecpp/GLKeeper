#include "pch.h"
#include "UnlitMeshComponentRenderer.h"
#include "GpuBuffer.h"

bool UnlitMeshComponentRenderer::Initialize()
{
    if (!mUnlitRenderProgram.LoadProgram())
    {
        debug_assert(false);
    }

    return true;
}

void UnlitMeshComponentRenderer::Deinit()
{
    mUnlitRenderProgram.FreeProgram();
}

void UnlitMeshComponentRenderer::Render(SceneRenderContext& renderContext, UnlitMeshComponent* component)
{
}

void UnlitMeshComponentRenderer::PrepareRenderdata(UnlitMeshComponent* component)
{
}

void UnlitMeshComponentRenderer::ReleaseRenderdata(UnlitMeshComponent* component)
{
}