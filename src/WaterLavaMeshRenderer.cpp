#include "pch.h"
#include "WaterLavaMeshRenderer.h"
#include "RenderScene.h"
#include "GraphicsDevice.h"
#include "WaterLavaMeshComponent.h"
#include "Cvars.h"

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

void WaterLavaMeshRenderer::Render(SceneRenderContext& renderContext, WaterLavaMeshComponent* component)
{
    if (!gCVarRender_DrawWaterAndLava.mValue)
        return;

    if (component == nullptr)
    {
        debug_assert(false);
        return;
    }

    if (renderContext.mCurrentPass == eRenderPass_Translucent && !component->mMaterial.IsTransparent())
        return;

    if (renderContext.mCurrentPass == eRenderPass_Opaque && component->mMaterial.IsTransparent())
        return;

    component->UpdateMesh();
    if (component->mVertexCount == 0)
    {
        debug_assert(false);
        return;
    }

    mWaterLavaRenderProgram.SetViewProjectionMatrix(gRenderScene.mCamera.mViewProjectionMatrix);
    mWaterLavaRenderProgram.SetTranslucency(component->mTranslucency);
    mWaterLavaRenderProgram.SetWaveParams(component->mWaveTime, component->mWaveWidth, component->mWaveHeight, component->mWaterlineHeight);
    mWaterLavaRenderProgram.ActivateProgram();

    // bind indices
    gGraphicsDevice.BindIndexBuffer(component->mIndicesBuffer);
    gGraphicsDevice.BindVertexBuffer(component->mVerticesBuffer, Vertex3D_WaterLava_Format::Get());

    component->mMaterial.ActivateMaterial();
    gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32, 0, component->mTriangleCount * 3);
}