#include "pch.h"
#include "AnimatingMeshRenderer.h"
#include "ModelAsset.h"
#include "GraphicsDevice.h"
#include "GpuBuffer.h"
#include "GameObjectComponent.h"
#include "AnimatingMeshComponent.h"
#include "RenderScene.h"
#include "VertexFormat.h"
#include "ConsoleVariable.h"
#include "cvars.h"
#include "GameObject.h"
#include "TransformComponent.h"

bool AnimatingMeshRenderer::Initialize()
{
    if (!mMorphAnimRenderProgram.LoadProgram())
    {
        debug_assert(false);
    }

    return true;
}

void AnimatingMeshRenderer::Deinit()
{
    mMorphAnimRenderProgram.FreeProgram();
    for (auto& curr_iterator : mModelsCache)
    {
        DestroyRenderData(&curr_iterator.second);
    }

    mModelsCache.clear();
}

void AnimatingMeshRenderer::Render(SceneRenderContext& renderContext, AnimatingMeshComponent* component)
{
    if (!gCVarRender_DrawModels.mValue)
        return;

    debug_assert(component);

    ModelAsset* modelAsset = component->mModelAsset;
    if (modelAsset == nullptr)
    {
        debug_assert(false);
        return;
    }

    if (component->mVertexBuffer == nullptr || component->mIndexBuffer == nullptr)
    {
        debug_assert(false);
        return;
    }

    TransformComponent* transformComponent = component->mGameObject->mTransformComponent;

    mMorphAnimRenderProgram.SetViewProjectionMatrix(gRenderScene.mCamera.mViewProjectionMatrix);
    mMorphAnimRenderProgram.SetModelMatrix(transformComponent->mTransformation);

    float mixFrames = component->mAnimState.mMixFrames;
    if (!gCvarRender_EnableAnimBlendFrames.mValue)
    {
        mixFrames = 0.0f;
    }
    mMorphAnimRenderProgram.SetMixFrames(mixFrames);
    mMorphAnimRenderProgram.ActivateProgram();

    // bind indices
    gGraphicsDevice.BindIndexBuffer(component->mIndexBuffer);

    Vertex3D_Anim_Format vertexDefs;
    for (size_t icurrSubset = 0, Count = modelAsset->mMeshArray.size(); 
        icurrSubset < Count; ++icurrSubset)
    {
        const ModelAsset::SubMesh& currentSubMesh = modelAsset->mMeshArray[icurrSubset];

        MeshMaterial* meshMaterial = component->GetMeshMaterial(currentSubMesh.mMaterialIndex);
        // filter out submeshes depending on current render pass
        if (renderContext.mCurrentPass == eRenderPass_Translucent && !meshMaterial->IsTransparent())
            continue;

        if (renderContext.mCurrentPass == eRenderPass_Opaque && meshMaterial->IsTransparent())
            continue;

        meshMaterial->ActivateMaterial();

        int frame0 = component->mAnimState.mFrame0;
        int frame1 = component->mAnimState.mFrame1;

        AnimatingMeshComponent::MeshPartStruct& currMeshPart = component->mMeshParts[icurrSubset];

        // prepare vertex streams definition
        vertexDefs.Setup(currMeshPart.mVertexDataOffset, currentSubMesh.mFrameVerticesCount, modelAsset->mFramesCount, frame0, frame1);
        gGraphicsDevice.BindVertexBuffer(component->mVertexBuffer, vertexDefs);
        gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32,
            currMeshPart.mIndexDataOffset, currMeshPart.mTriangleCount * 3);
    }
}

AnimModelRenderdata* AnimatingMeshRenderer::GetRenderData(ModelAsset* modelAsset)
{
    if (modelAsset == nullptr || !modelAsset->IsModelLoaded())
    {
        debug_assert(false);
        return nullptr;
    }

    auto cache_iterator = mModelsCache.find(modelAsset);
    if (cache_iterator != mModelsCache.end())
        return &cache_iterator->second;

    AnimModelRenderdata* renderdata = &mModelsCache[modelAsset];

    // upload geometry
    InitRenderData(renderdata, modelAsset);
    return renderdata;
}

void AnimatingMeshRenderer::InvalidateRenderData(ModelAsset* modelAsset)
{
    if (modelAsset == nullptr || !modelAsset->IsModelLoaded())
    {
        debug_assert(false);
        return;
    }

    auto cache_iterator = mModelsCache.find(modelAsset);
    if (cache_iterator == mModelsCache.end())
        return;

    // upload geometry
    InitRenderData(&cache_iterator->second, modelAsset);
}

void AnimatingMeshRenderer::DestroyRenderData(AnimModelRenderdata* renderdata)
{
    debug_assert(renderdata);
    if (renderdata->mVertexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(renderdata->mVertexBuffer);
    }
    if (renderdata->mIndexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(renderdata->mIndexBuffer);
    }
    renderdata->Clear();
}

void AnimatingMeshRenderer::InitRenderData(AnimModelRenderdata* renderdata, ModelAsset* modelAsset)
{
    debug_assert(renderdata);
    debug_assert(modelAsset);

    renderdata->mSubsets.resize(modelAsset->mMeshArray.size());
    for (size_t icurrSubset = 0; icurrSubset < modelAsset->mMeshArray.size(); ++icurrSubset)
    {
        renderdata->mSubsets[icurrSubset].mSubsetLODs.resize(
            modelAsset->mMeshArray[icurrSubset].mLODsArray.size());
    }

    // compute required size of vertex and index buffers
    int numVerticesPerFrame = 0;
    int numTriangles = 0;
    for (ModelAsset::SubMesh& currentSubMesh: modelAsset->mMeshArray)
    {
        numVerticesPerFrame += currentSubMesh.mFrameVerticesCount;
        for (ModelAsset::SubMeshLOD& currentLOD: currentSubMesh.mLODsArray)
        {
            int trianglesInLOD = currentLOD.mTriangleArray.size();
            numTriangles += trianglesInLOD;
        }
    }

    int vbufferLengthBytes = (numVerticesPerFrame * modelAsset->mFramesCount * (sizeof(glm::vec3) + sizeof(glm::vec3))) +
        (numVerticesPerFrame * sizeof(glm::vec2));

    debug_assert(vbufferLengthBytes > 0);

    renderdata->mVertexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices, eBufferUsage_Static, vbufferLengthBytes, nullptr);
    debug_assert(renderdata->mVertexBuffer);

    // upload vertex attributes
    unsigned char* vbufferptr = (unsigned char*)renderdata->mVertexBuffer->Lock(BufferAccess_UnsynchronizedWrite);
    debug_assert(vbufferptr);
    if (vbufferptr)
    {
        // attributes layout:
        // submesh #0 - [texture coords] [positions of all animation frames] [normals of all animation frames]
        // submesh #1 - [texture coords] [positions of all animation frames] [normals of all animation frames]
        // and so on

        int currentBufferOffset = 0;
        for (size_t icurrSubset = 0; icurrSubset < modelAsset->mMeshArray.size(); ++icurrSubset)
        {
            ModelAsset::SubMesh& currentSubMesh = modelAsset->mMeshArray[icurrSubset];
            renderdata->mSubsets[icurrSubset].mVertexDataOffset = currentBufferOffset;

            int texcoordsDataLength = currentSubMesh.mVertexTexCoordArray.size() * sizeof(glm::vec2);
            ::memcpy(vbufferptr + currentBufferOffset, currentSubMesh.mVertexTexCoordArray.data(), texcoordsDataLength);
            currentBufferOffset += texcoordsDataLength;

            int positionsDataLength = currentSubMesh.mVertexPositionArray.size() * sizeof(glm::vec3);
            ::memcpy(vbufferptr + currentBufferOffset, currentSubMesh.mVertexPositionArray.data(), positionsDataLength);
            currentBufferOffset += positionsDataLength;

            int normalsDataLength = currentSubMesh.mVertexNormalArray.size() * sizeof(glm::vec3);
            ::memcpy(vbufferptr + currentBufferOffset, currentSubMesh.mVertexNormalArray.data(), normalsDataLength);
            currentBufferOffset += normalsDataLength;
        }

        if (!renderdata->mVertexBuffer->Unlock())
        {
            debug_assert(false);
        }
    } // if

    int ibufferLengthByets = numTriangles * sizeof(glm::ivec3);
    debug_assert(ibufferLengthByets > 0);

    renderdata->mIndexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Indices, eBufferUsage_Static, ibufferLengthByets, nullptr);
    debug_assert(renderdata->mIndexBuffer);

    // upload index data
    unsigned char* ibufferptr = (unsigned char*)renderdata->mIndexBuffer->Lock(BufferAccess_UnsynchronizedWrite);
    debug_assert(ibufferptr);
    if (ibufferptr)
    {
        int currentBufferOffset = 0;
        for (size_t icurrSubset = 0; icurrSubset < modelAsset->mMeshArray.size(); ++icurrSubset)
        {
            ModelAsset::SubMesh& currentSubMesh = modelAsset->mMeshArray[icurrSubset];
            renderdata->mSubsets[icurrSubset].mIndexDataOffset = currentBufferOffset;
            for (size_t icurrLOD = 0; icurrLOD < currentSubMesh.mLODsArray.size(); ++icurrLOD)
            {
                ModelAsset::SubMeshLOD& currentLOD = currentSubMesh.mLODsArray[icurrLOD];
                renderdata->mSubsets[icurrSubset].mSubsetLODs[icurrLOD].mIndexDataOffset = currentBufferOffset;
                int currentLODDataLength = currentLOD.mTriangleArray.size() * sizeof(glm::ivec3);
                if (currentLODDataLength < 1)
                    continue;

                ::memcpy(ibufferptr + currentBufferOffset, currentLOD.mTriangleArray.data(), currentLODDataLength);
                currentBufferOffset += currentLODDataLength;
            }
        }

        if (!renderdata->mIndexBuffer->Unlock())
        {
            debug_assert(false);
        }
    } // if
}
