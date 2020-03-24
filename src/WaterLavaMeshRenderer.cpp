#include "pch.h"
#include "WaterLavaMeshRenderer.h"
#include "WaterLavaMesh.h"
#include "RenderScene.h"
#include "GraphicsDevice.h"

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
    if (waterLavaMesh == nullptr || waterLavaMesh->mVertexCount == 0)
    {
        debug_assert(false);
        return;
    }

    if (renderContext.mCurrentPass == eRenderPass_Translucent && !waterLavaMesh->mMaterial.IsTransparent())
        return;

    if (renderContext.mCurrentPass == eRenderPass_Opaque && waterLavaMesh->mMaterial.IsTransparent())
        return;

    mWaterLavaRenderProgram.SetViewProjectionMatrix(gRenderScene.mCamera.mViewProjectionMatrix);
    mWaterLavaRenderProgram.SetTranslucency(waterLavaMesh->mTranslucency);
    mWaterLavaRenderProgram.SetWaveParams(waterLavaMesh->mWaveTime, waterLavaMesh->mWaveWidth, waterLavaMesh->mWaveHeight, waterLavaMesh->mWaterlineHeight);
    mWaterLavaRenderProgram.ActivateProgram();

    // bind indices
    gGraphicsDevice.BindIndexBuffer(waterLavaMesh->mIndicesBuffer);
    gGraphicsDevice.BindVertexBuffer(waterLavaMesh->mVerticesBuffer, Vertex3D_WaterLava_Format::Get());

    waterLavaMesh->mMaterial.ActivateMaterial();
    gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32, 0, waterLavaMesh->mTriangleCount * 3);
}