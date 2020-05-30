#pragma once

// forwards
class DebugRenderer;
class RenderManager;
class RenderProgram;
class MeshMaterial;

enum eRenderPass
{
    eRenderPass_Opaque,
    eRenderPass_Translucent,

    eRenderPass_Count,
};

decl_enum_strings(eRenderPass);

// render context
class SceneRenderContext
{
public:
    SceneRenderContext() = default;

public:
    eRenderPass mCurrentPass;
};

// anim model cached render data, it is managed by AnimModelsRenderer
class ModelAssetRenderdata
{
public:
    ModelAssetRenderdata() = default;

    // reset renderdata to initial state
    inline void Clear()
    {
        mVertexBuffer = nullptr;
        mIndexBuffer = nullptr;
        mSubsets.clear();
        mSubsetMaterials.clear();
    }
public:
    struct SubsetLOD
    {
    public:
        int mIndexDataOffset = 0;
    };
    struct Subset
    {
    public:
        std::vector<SubsetLOD> mSubsetLODs;
        int mVertexDataOffset = 0;
        int mIndexDataOffset = 0;
    };
public:
    std::vector<Subset> mSubsets;
    std::vector<MeshMaterial> mSubsetMaterials;
    GpuBuffer* mVertexBuffer = nullptr;
    GpuBuffer* mIndexBuffer = nullptr;
};
