#include "pch.h"
#include "AnimatingModelsRenderer.h"
#include "ModelAsset.h"
#include "GraphicsDevice.h"
#include "GpuBuffer.h"
#include "SceneObjectComponent.h"
#include "AnimatingModelComponent.h"
#include "RenderScene.h"
#include "VertexFormat.h"
#include "ConsoleVariable.h"
#include "cvars.h"
#include "SceneObject.h"

// internal info
class ModelsRenderData: public cxx::noncopyable
{
public:

    struct SubsetLOD
    {
    public:
        int mIndicesDataOffset = 0;
    };

    struct Subset
    {
    public:
        std::vector<SubsetLOD> mSubsetLODs;
        int mVerticesDataOffset = 0;
        int mIndicesDataOffset = 0;
    };

public:
    std::vector<Subset> mSubsets;

    GpuBuffer* mVerticesBuffer = nullptr;
    GpuBuffer* mIndicesBuffer = nullptr;
};

//////////////////////////////////////////////////////////////////////////

bool AnimatingModelsRenderer::Initialize()
{
    if (!mMorphAnimRenderProgram.LoadProgram())
    {
        debug_assert(false);
    }

    return true;
}

void AnimatingModelsRenderer::Deinit()
{
    mMorphAnimRenderProgram.FreeProgram();
    for (auto& curr_iterator : mModelsCache)
    {
        DestroyRenderData(curr_iterator.second);
    }

    mModelsCache.clear();
}

void AnimatingModelsRenderer::Render(SceneRenderContext& renderContext, AnimatingModelComponent* component)
{
    if (!gCVarRender_DrawModels.mValue)
        return;

    if (component == nullptr || component->mModelAsset == nullptr)
    {
        debug_assert(false);
        return;

    }
    ModelAsset* modelAsset = component->mModelAsset;
    if (component->mRenderData == nullptr)
    {
        component->mRenderData = GetRenderData(modelAsset);
        if (component->mRenderData == nullptr)
        {
            debug_assert(false);
            return;
        }
    }

    SceneObjectTransform* transformComponent = component->mSceneObject->GetTransformComponent();

    ModelsRenderData* renderData = component->mRenderData;

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
    gGraphicsDevice.BindIndexBuffer(renderData->mIndicesBuffer);

    // select level of details to render
    int selectLOD = component->mPreferredLOD;

    Vertex3D_Anim_Format vertexDefs;

    for (size_t icurrSubset = 0, Count = modelAsset->mMeshArray.size(); 
        icurrSubset < Count; ++icurrSubset)
    {
        const ModelAsset::SubMesh& currentSubMesh = modelAsset->mMeshArray[icurrSubset];
        // if submesh does not have geoemtry for selected lod, then dont draw it
        if (selectLOD >= (int)currentSubMesh.mLODsArray.size())
            continue;

        RenderMaterial& renderMaterial = component->mSubmeshMaterials[currentSubMesh.mMaterialIndex];
        // filter out submeshes depending on current render pass
        if (renderContext.mCurrentPass == eRenderPass_Translucent && !renderMaterial.IsTransparent())
            continue;

        if (renderContext.mCurrentPass == eRenderPass_Opaque && renderMaterial.IsTransparent())
            continue;

        renderMaterial.ActivateMaterial();

        int frame0 = component->mAnimState.mFrame0;
        int frame1 = component->mAnimState.mFrame1;

        // prepare vertex streams definition
        vertexDefs.Setup(renderData->mSubsets[icurrSubset].mVerticesDataOffset, currentSubMesh.mFrameVerticesCount, modelAsset->mFramesCount, frame0, frame1);
        gGraphicsDevice.BindVertexBuffer(renderData->mVerticesBuffer, vertexDefs);
        gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32,
            renderData->mSubsets[icurrSubset].mSubsetLODs[selectLOD].mIndicesDataOffset,
            currentSubMesh.mLODsArray[selectLOD].mTriangleArray.size() * 3);
    }
}

ModelsRenderData* AnimatingModelsRenderer::GetRenderData(ModelAsset* modelAsset)
{
    if (modelAsset == nullptr || !modelAsset->IsModelLoaded())
    {
        debug_assert(false);
        return nullptr;
    }

    auto cache_iterator = mModelsCache.find(modelAsset);
    if (cache_iterator != mModelsCache.end())
        return cache_iterator->second;

    ModelsRenderData* renderdata = new ModelsRenderData;
    mModelsCache[modelAsset] = renderdata;

    // upload geometry
    InitRenderData(renderdata, modelAsset);
    return renderdata;
}

void AnimatingModelsRenderer::InvalidateRenderData(ModelAsset* modelAsset)
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
    InitRenderData(cache_iterator->second, modelAsset);
}

void AnimatingModelsRenderer::DestroyRenderData(ModelsRenderData* renderdata)
{
    debug_assert(renderdata);
    if (renderdata->mVerticesBuffer)
    {
        gGraphicsDevice.DestroyBuffer(renderdata->mVerticesBuffer);
    }
    if (renderdata->mIndicesBuffer)
    {
        gGraphicsDevice.DestroyBuffer(renderdata->mIndicesBuffer);
    }
    SafeDelete(renderdata);
}

void AnimatingModelsRenderer::InitRenderData(ModelsRenderData* renderdata, ModelAsset* modelAsset)
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

    renderdata->mVerticesBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices, eBufferUsage_Static, vbufferLengthBytes, nullptr);
    debug_assert(renderdata->mVerticesBuffer);

    // upload vertex attributes
    unsigned char* vbufferptr = (unsigned char*)renderdata->mVerticesBuffer->Lock(BufferAccess_UnsynchronizedWrite);
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
            renderdata->mSubsets[icurrSubset].mVerticesDataOffset = currentBufferOffset;

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

        if (!renderdata->mVerticesBuffer->Unlock())
        {
            debug_assert(false);
        }
    } // if

    int ibufferLengthByets = numTriangles * sizeof(glm::ivec3);
    debug_assert(ibufferLengthByets > 0);

    renderdata->mIndicesBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Indices, eBufferUsage_Static, ibufferLengthByets, nullptr);
    debug_assert(renderdata->mIndicesBuffer);

    // upload index data
    unsigned char* ibufferptr = (unsigned char*)renderdata->mIndicesBuffer->Lock(BufferAccess_UnsynchronizedWrite);
    debug_assert(ibufferptr);
    if (ibufferptr)
    {
        int currentBufferOffset = 0;
        for (size_t icurrSubset = 0; icurrSubset < modelAsset->mMeshArray.size(); ++icurrSubset)
        {
            ModelAsset::SubMesh& currentSubMesh = modelAsset->mMeshArray[icurrSubset];
            renderdata->mSubsets[icurrSubset].mIndicesDataOffset = currentBufferOffset;
            for (size_t icurrLOD = 0; icurrLOD < currentSubMesh.mLODsArray.size(); ++icurrLOD)
            {
                ModelAsset::SubMeshLOD& currentLOD = currentSubMesh.mLODsArray[icurrLOD];
                renderdata->mSubsets[icurrSubset].mSubsetLODs[icurrLOD].mIndicesDataOffset = currentBufferOffset;
                int currentLODDataLength = currentLOD.mTriangleArray.size() * sizeof(glm::ivec3);
                if (currentLODDataLength < 1)
                    continue;

                ::memcpy(ibufferptr + currentBufferOffset, currentLOD.mTriangleArray.data(), currentLODDataLength);
                currentBufferOffset += currentLODDataLength;
            }
        }

        if (!renderdata->mIndicesBuffer->Unlock())
        {
            debug_assert(false);
        }
    } // if
}
