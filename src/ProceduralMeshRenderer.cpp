#include "stdafx.h"
#include "ProceduralMeshRenderer.h"
#include "ShadersManager.h"
#include "GameWorld.h"

bool ProceduralMeshRenderer::Initialize()
{
    mShaderProgram = gShadersManager.GetProgramOfType<ShaderProgram_StaticMesh>("mesh_static");
    cxx_assert(mShaderProgram);

    if (mShaderProgram == nullptr)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load mesh_static shader program");
        return false;
    }

    mShaderProgram->InitRenderData();

    return true;
}

void ProceduralMeshRenderer::Shutdown()
{
    mShaderProgram = nullptr;
}

void ProceduralMeshRenderer::BeginFrame(Camera& camera)
{
    // prepare render program
    mShaderProgram->SetViewProjectionMatrix(camera.mViewProjectionMatrix);
}

void ProceduralMeshRenderer::EndFrame()
{
}

void ProceduralMeshRenderer::BeginBatch()
{
    mShaderProgram->BindProgram();
}

void ProceduralMeshRenderer::EndBatch()
{
    
}

void ProceduralMeshRenderer::RenderInstance(eRenderPass currentPass, ProceduralMeshObject& object)
{
    // validate render pass
    cxx_assert(object.mRenderMaterial.IsOpaque() ? (currentPass == eRenderPass_Opaque) : (currentPass == eRenderPass_Translucent));

    object.PrepareRenderdata();
    if (object.mUploadedVertsCount == 0) return;

    mShaderProgram->SetModelMatrix(object.mTransformation);
    object.mRenderMaterial.BindMaterial(*mShaderProgram);

    // bind vertex attributes
    gRenderDevice.BindVertexBuffer(object.mGpuVertexBuffer.get());
    if (object.mUploadedTrisCount > 0)
    {
        gRenderDevice.BindIndexBuffer(object.mGpuIndexBuffer.get());
        gRenderDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32, 0, object.mUploadedTrisCount * 3);
    }
    else
    {
        gRenderDevice.RenderPrimitives(ePrimitiveType_Triangles, 0, object.mUploadedVertsCount);
    }
    
}
