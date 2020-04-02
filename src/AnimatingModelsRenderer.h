#pragma once

#include "ResourceDefs.h"
#include "RenderDefs.h"
#include "MorphAnimRenderProgram.h"
#include "SceneDefs.h"
#include "GameDefs.h"

// models visualization manager
class AnimatingModelsRenderer: public cxx::noncopyable
{
public:

    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // render animating model for current render pass
    // @param renderContext: Current render context
    // @param component: Renderable component
    void Render(SceneRenderContext& renderContext, AnimModelComponent* component);

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