#pragma once

#include "SceneDefs.h"
#include "RenderDefs.h"
#include "AnimatingModel.h"

// list for collecting scene entities which will be rendered on current frame
class SceneRenderList
{
public:
    SceneRenderList() = default;

    static const int MaxElements = 16383;
    inline void RegisterRenderable(eRenderPass renderPass, AnimatingModel* animatingModel)
    {
        debug_assert(animatingModel);
        if (renderPass == eRenderPass_Opaque)
        {
            if (mOpaqueElementsCount == MaxElements)
                return;

            mOpaqueElements[mOpaqueElementsCount++].Set(animatingModel);
            return;
        }

        if (renderPass == eRenderPass_Translucent)
        {
            if (mTranslucentElementsCount == MaxElements)
                return;

            mTranslucentElements[mTranslucentElementsCount++].Set(animatingModel);
            return;
        }
        debug_assert(false);
    }

    // discard all previously collected entities
    inline void Clear()
    {
        mOpaqueElementsCount = 0;
        mTranslucentElementsCount = 0;
    }

public:

    struct ListElement
    {
    public:
        inline void Set(AnimatingModel* animatingModel)
        {
            mRenderable = animatingModel;
            mAnimatingModel = animatingModel;
        }

    public:
        Renderable* mRenderable = nullptr;
        AnimatingModel* mAnimatingModel = nullptr;
    };

    ListElement mOpaqueElements[MaxElements];
    int mOpaqueElementsCount = 0;

    ListElement mTranslucentElements[MaxElements];
    int mTranslucentElementsCount = 0;
};
