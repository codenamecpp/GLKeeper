#pragma once

#include "SceneDefs.h"
#include "RenderDefs.h"
#include "SceneObjectComponent.h"

// list for collecting scene entities which will be rendered on current frame
class SceneRenderList
{
private:
    static const int MaxElements = 8192;

public:
    SceneRenderList() = default;

    inline void RegisterRenderableComponent(eRenderPass renderPass, SceneObjectComponent* component)
    {
        debug_assert(component);

        ComponentsList& renderPassList = mComponentsForRenderPass[renderPass];
        if (renderPassList.mElementsCount < MaxElements)
        {
            renderPassList.mElements[renderPassList.mElementsCount++] = component;
        }        
    }

    // discard all previously collected entities
    inline void Clear()
    {
        for (int icurr = 0; icurr < eRenderPass_Count; ++icurr)
        {
            mComponentsForRenderPass[icurr].mElementsCount = 0;
        }
    }

    // sort opaque objects by render program
    void SortOpaqueComponents();

    // sort translucent object by distance to camera
    void SortTranslucentComponents();

public:
    // registered components lists
    struct ComponentsList
    {
        int mElementsCount = 0;

        SceneObjectComponent* mElements[MaxElements];
    };

    ComponentsList mComponentsForRenderPass[eRenderPass_Count];
};
