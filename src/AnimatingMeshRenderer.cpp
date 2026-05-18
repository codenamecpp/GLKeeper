#include "stdafx.h"
#include "AnimatingMeshRenderer.h"
#include "GameRenderManager.h"
#include "GameWorld.h"
#include "ShadersManager.h"

bool AnimatingMeshRenderer::Initialize()
{
    mShaderProgram = gShadersManager.GetProgramOfType<ShaderProgram_BlendFrames>("mesh_blend_frames");
    cxx_assert(mShaderProgram);
    if (mShaderProgram == nullptr)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load mesh_blend_frames shader program");
        return false;
    }
    mShaderProgram->InitRenderData();
    return true;
}

void AnimatingMeshRenderer::Shutdown()
{
}

void AnimatingMeshRenderer::BeginFrame(Camera& camera)
{    
    mShaderProgram->SetViewProjectionMatrix(camera.mViewProjectionMatrix);
}

void AnimatingMeshRenderer::EndFrame()
{
}

void AnimatingMeshRenderer::BeginBatch()
{
    mShaderProgram->BindProgram();
}

void AnimatingMeshRenderer::EndBatch()
{

}

void AnimatingMeshRenderer::RenderInstance(eRenderPass currentPass, AnimatingMeshObject& object)
{
    MeshAsset* meshAsset = object.mMeshAsset;
    cxx_assert(meshAsset);

    if (!meshAsset->IsRenderDataInited())
    {
        meshAsset->InitRenderData();
    }

    mShaderProgram->SetMixFrames(object.mAnimState.mFramesMixFactor);
    mShaderProgram->SetModelMatrix(object.mTransformation);
    
    // bind indices
    gRenderDevice.BindIndexBuffer(meshAsset->GetGpuIndexBuffer());

    AnimVertex3D_Format vertexDefs;
    for (const MeshAsset::SubMesh& currentSubMesh: meshAsset->GetSubMeshList())
    {
        const SurfaceMaterial& renderMaterial = meshAsset->GetRenderMaterial(currentSubMesh.mMaterialIndex);

        // filter out submeshes depending on current render pass
        if (currentPass == eRenderPass_Translucent && renderMaterial.IsOpaque())
            continue;

        if (currentPass == eRenderPass_Opaque && !renderMaterial.IsOpaque())
            continue;

        renderMaterial.BindMaterial(*mShaderProgram);

        int frame0 = object.mAnimState.mCurrFrame;
        int frame1 = object.ComputeNextAnimationFrame();

        // prepare vertex streams definition
        vertexDefs.Setup(currentSubMesh.mGpuVBufferOffset, currentSubMesh.mFrameVerticesCount, meshAsset->GetAnimFramesCount(), frame0, frame1);
        gRenderDevice.BindVertexBuffer(meshAsset->GetGpuVertexBuffer(), vertexDefs);
        gRenderDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32,
            currentSubMesh.mGpuIBufferOffset, 
            currentSubMesh.mTris.size() * 3);
    }
}