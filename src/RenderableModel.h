#pragma once

#include "GraphicsDefs.h"
#include "RenderDefs.h"
#include "RenderMaterial.h"
#include "ModelAsset.h"

// render data specific for animated models
class RenderableModel: public cxx::noncopyable
{
    friend class ModelsRenderer;

private:
    RenderableModel();
    ~RenderableModel();

    // upload geometry data to gpu, setup render materials etc
    // sourceModel: Source model
    bool InitFromData(const ModelAsset* sourceModel);
    void Free();

    // draw all model subsets for current renderpass with blending frames
    void DrawSubsetsForCurrentRenderPass(eRenderPass renderPass, int animFrame0, int animFrame1);

private:

    struct SubsetRecord
    {
    public:
        int mFrameVerticesCount = 0;
        int mTrianglesCount = 0;
        int mVerticesDataOffset = 0;
        int mIndicesDataOffset = 0;
        int mMaterialIndex = 0;
    };
    std::vector<SubsetRecord> mSubsets;
    std::vector<RenderMaterial> mMaterials;
    std::vector<cxx::aabbox> mFramesBounds;
    int mAnimFramesCount = 0;
    GpuBuffer* mVerticesBuffer = nullptr;
    GpuBuffer* mIndicesBuffer = nullptr;
};