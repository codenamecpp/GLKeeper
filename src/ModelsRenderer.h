#pragma once

#include "RenderableModel.h"

// models visualization manager
class ModelsRenderer: public cxx::noncopyable
{
public:

    // setup renderer internal resources
    bool Initialize();
    void Deinit();

    RenderableModel* GetRenderableModel(const ModelAsset* sourceModel)
    {
        return nullptr;
    }

    void InvalidateRenderableModel(const ModelAsset* sourceModel);

private:

};