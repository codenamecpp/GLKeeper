#include "pch.h"
#include "ModelsRenderer.h"
#include "ModelAsset.h"
#include "GraphicsDevice.h"
#include "GpuBuffer.h"
#include "AnimatingModel.h"
#include "RenderScene.h"
#include "VertexFormat.h"

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

bool ModelsRenderer::Initialize()
{
    if (!mMorphAnimRenderProgram.LoadProgram())
    {
        Deinit();
        return false;
    }
    return true;
}

void ModelsRenderer::Deinit()
{
    mMorphAnimRenderProgram.FreeProgram();
    for (auto& curr_iterator : mModelsCache)
    {
        DestroyRenderData(curr_iterator.second);
    }
    mModelsCache.clear();
}

void ModelsRenderer::RenderModel(eRenderPass renderPass, AnimatingModel* animatingModel)
{
    ModelAsset* modelAsset = animatingModel->mModelAsset;
    if (animatingModel == nullptr || modelAsset== nullptr)
    {
        debug_assert(false);
        return;
    }

    if (animatingModel->mRenderData == nullptr)
    {
        animatingModel->mRenderData = GetRenderData(modelAsset);
        if (animatingModel->mRenderData == nullptr)
        {
            debug_assert(false);
            return;
        }
    }
    ModelsRenderData* renderData = animatingModel->mRenderData;

    mMorphAnimRenderProgram.SetViewProjectionMatrix(gRenderScene.mCamera.mViewProjectionMatrix);
    mMorphAnimRenderProgram.SetModelMatrix(animatingModel->mTransformation);
    mMorphAnimRenderProgram.SetMixFrames(0.0f);
    mMorphAnimRenderProgram.ActivateProgram();

    // bind indices
    gGraphicsDevice.BindIndexBuffer(renderData->mIndicesBuffer);

    // select level of details to render
    int selectLOD = 0;

    Vertex3D_Anim_Format vertexDefs;

    for (size_t icurrSubset = 0, Count = modelAsset->mMeshArray.size(); 
        icurrSubset < Count; ++icurrSubset)
    {
        const ModelAsset::SubMesh& currentSubMesh = modelAsset->mMeshArray[icurrSubset];
        // if submesh does not have geoemtry for selected lod, then dont draw it
        if (selectLOD >= (int)currentSubMesh.mLODsArray.size())
            continue;

        RenderMaterial& renderMaterial = animatingModel->mSubmeshMaterials[currentSubMesh.mMaterialIndex];
        // filter out submeshes depending on current render pass
        if (renderPass == eRenderPass_Translucent && !renderMaterial.IsTransparent())
            continue;

        if (renderPass == eRenderPass_Opaque && renderMaterial.IsTransparent())
            continue;

        renderMaterial.ActivateMaterial();

        int frame0 = 0;
        int frame1 = 0;

        // prepare vertex streams definition
        vertexDefs.Setup(renderData->mSubsets[icurrSubset].mVerticesDataOffset, currentSubMesh.mFrameVerticesCount, modelAsset->mFramesCount, frame0, frame1);
        gGraphicsDevice.BindVertexBuffer(renderData->mVerticesBuffer, vertexDefs);
        gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32,
            renderData->mSubsets[icurrSubset].mSubsetLODs[selectLOD].mIndicesDataOffset,
            currentSubMesh.mLODsArray[selectLOD].mTriangleArray.size() * 3);
    }
}

ModelsRenderData* ModelsRenderer::GetRenderData(ModelAsset* modelAsset)
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

void ModelsRenderer::InvalidateRenderData(ModelAsset* modelAsset)
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

void ModelsRenderer::DestroyRenderData(ModelsRenderData* renderdata)
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

void ModelsRenderer::InitRenderData(ModelsRenderData* renderdata, ModelAsset* modelAsset)
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
