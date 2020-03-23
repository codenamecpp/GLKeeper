#include "pch.h"
#include "WaterLavaMeshRenderer.h"
#include "WaterLavaMesh.h"

bool WaterLavaMeshRenderer::Initialize()
{
    if (!mWaterLavaRenderProgram.LoadProgram())
    {
        debug_assert(false);
    }
    return true;
}

void WaterLavaMeshRenderer::Deinit()
{
    mWaterLavaRenderProgram.FreeProgram();
}

void WaterLavaMeshRenderer::RenderWaterLavaMesh(SceneRenderContext& renderContext, WaterLavaMesh* waterLavaMesh)
{
}