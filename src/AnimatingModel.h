#pragma once

#include "SceneDefs.h"
#include "ResourceDefs.h"
#include "Renderable.h"
#include "RenderDefs.h"

// model instance on render scene
class AnimatingModel: public Renderable
{
    friend class ModelsRenderer;

public:
    // readonly
    ModelAsset* mModelAsset = nullptr;

    std::vector<RenderMaterial> mSubmeshMaterials;

public:
    AnimatingModel();
    ~AnimatingModel();

    // change model asset, setup bounds and materials
    // @param modelAsset: Source model data
    void SetModelAsset(ModelAsset* modelAsset);
    void SetModelAssetNull();

    // request entity to register itself in render lists
    // @param renderPass: Current render pass
    // @param renderList: Render lists
    void RegisterForRendering(SceneRenderList& renderList) override;

private:
    ModelsRenderData* mRenderData = nullptr;
};