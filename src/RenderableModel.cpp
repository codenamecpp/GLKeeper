#include "pch.h"
#include "RenderableModel.h"
#include "GpuBuffer.h"
#include "VertexFormat.h"
#include "GraphicsDevice.h"

RenderableModel::RenderableModel()
{
}

RenderableModel::~RenderableModel()
{
    Free();
}

bool RenderableModel::InitFromData(const ModelAsset* sourceModel)
{
    if (sourceModel == nullptr)
    {
        debug_assert(false);
        return false;
    }

    Free();

    mAnimFramesCount = sourceModel->mFramesCount;
    mFramesBounds = sourceModel->mFramesBounds;
    mSubsets.resize(sourceModel->mMeshArray.size());
    mMaterials.resize(sourceModel->mMaterialsArray.size());

    // todo
    return true;
}

void RenderableModel::Free()
{
    // todo
}

void RenderableModel::DrawSubsetsForCurrentRenderPass(eRenderPass renderPass, int frame0, int frame1)
{
    gGraphicsDevice.BindIndexBuffer(mIndicesBuffer);

    AnimVertex3D_Format vertexDefs;
    for (const SubsetRecord& currentSubset: mSubsets)
    {
        RenderMaterial& renderMaterial = mMaterials[currentSubset.mMaterialIndex];
        // filter out subsets depending on current render pass
        if (renderPass == eRenderPass_Translucent && !renderMaterial.IsTransparent())
            continue;

        if (renderPass == eRenderPass_Opaque && renderMaterial.IsTransparent())
            continue;

        renderMaterial.ActivateMaterial();

        // prepare vertex streams definition
        vertexDefs.Setup(currentSubset.mVerticesDataOffset, currentSubset.mFrameVerticesCount, mAnimFramesCount, frame0, frame1);
        gGraphicsDevice.BindVertexBuffer(mVerticesBuffer, vertexDefs);
        gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32, currentSubset.mIndicesDataOffset, currentSubset.mTrianglesCount * 3);
    }
}
