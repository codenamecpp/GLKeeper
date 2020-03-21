#pragma once

#include "ResourceDefs.h"
#include "RenderDefs.h"
#include "MorphAnimRenderProgram.h"
#include "SceneDefs.h"

// models visualization manager
class ModelsRenderer: public cxx::noncopyable
{
public:

    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render animating model for current render pass
    // @param renderContext: Current render context
    // @param animatingModel: Model instance
    void RenderModel(SceneRenderContext& renderContext, AnimatingModel* animatingModel);

    // recreate renderdata for specific model asset
    // @param modelAsset: Model data
    void InvalidateRenderData(ModelAsset* modelAsset);

private:
    // create renderdata for model asset and put it in internal cache
    // @param modelAsset: Model data
    ModelsRenderData* GetRenderData(ModelAsset* modelAsset);

    void DestroyRenderData(ModelsRenderData* renderdata);
    void InitRenderData(ModelsRenderData* renderdata, ModelAsset* modelAsset);

private:
    std::map<ModelAsset*, ModelsRenderData*> mModelsCache;
    MorphAnimRenderProgram mMorphAnimRenderProgram;
};