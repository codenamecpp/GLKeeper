#pragma once

#include "MorphAnimRenderProgram.h"

// animating models visualization manager
class AnimatingMeshRenderer: public cxx::noncopyable
{
    friend class AnimatingMeshComponent;

public:
    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render animating model for current render pass
    // @param renderContext: Current render context
    // @param component: Renderable component
    void Render(SceneRenderContext& renderContext, AnimatingMeshComponent* component);

    // recreate renderdata for specific model asset
    // @param modelAsset: Model data
    void InvalidateRenderData(ModelAsset* modelAsset);

private:
    // create renderdata for model asset and put it in internal cache
    // @param modelAsset: Model data
    AnimModelRenderdata* GetRenderData(ModelAsset* modelAsset);

    void InitRenderData(AnimModelRenderdata* renderdata, ModelAsset* modelAsset);
    void DestroyRenderData(AnimModelRenderdata* renderdata);

private:
    MorphAnimRenderProgram mMorphAnimRenderProgram;
    std::map<ModelAsset*, AnimModelRenderdata> mModelsCache;
};