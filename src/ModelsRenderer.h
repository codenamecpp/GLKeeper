#pragma once

#include "ResourceDefs.h"
#include "RenderDefs.h"
#include "MorphAnimRenderProgram.h"

// models visualization manager
class ModelsRenderer: public cxx::noncopyable
{
public:

    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    // create renderdata for model asset and put it in internal cache
    // @param modelAsset: Model data
    ModelsRenderData* GetRenderData(ModelAsset* modelAsset);

    // recreate renderdata for specific model asset
    // @param modelAsset: Model data
    void InvalidateRenderData(ModelAsset* modelAsset);

private:
    void DestroyRenderData(ModelsRenderData* renderdata);
    void InitRenderData(ModelsRenderData* renderdata, ModelAsset* modelAsset);

private:
    std::map<ModelAsset*, ModelsRenderData*> mModelsCache;
    MorphAnimRenderProgram mMorphAnimRenderProgram;
};