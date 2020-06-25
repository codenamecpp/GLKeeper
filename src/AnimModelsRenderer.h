#pragma once

#include "Shaders.h"

// animating models visualization manager
class AnimModelsRenderer: public cxx::noncopyable
{
    friend class RenderableModel;

public:
    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render animating model for current render pass
    // @param renderContext: Current render context
    // @param component: Renderable component
    void Render(SceneRenderContext& renderContext, RenderableModel* component);

    // recreate renderdata for specific model asset
    // @param modelAsset: Model data
    void InvalidateRenderData(ModelAsset* modelAsset);

private:
    // setup component renderdata
    void PrepareRenderdata(RenderableModel* component);
    void ReleaseRenderdata(RenderableModel* component);

    // create renderdata for model asset and put it in internal cache
    // @param modelAsset: Model data
    ModelAssetRenderdata* GetRenderdata(ModelAsset* modelAsset);

    void PrepareRenderdata(ModelAssetRenderdata* renderdata, ModelAsset* modelAsset);
    void ReleaseRenderdata(ModelAssetRenderdata* renderdata);

private:
    MorphAnimRenderProgram mMorphAnimRenderProgram;
    std::map<ModelAsset*, ModelAssetRenderdata> mModelsCache;
};