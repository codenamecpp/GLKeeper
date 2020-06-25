#pragma once

#include "GameDefs.h"

// list for collecting scene objects which will be rendered on current frame
class SceneRenderList
{
private:
    static const int MaxElements = 8192;

public:
    SceneRenderList() = default;

    inline void RegisterSceneObject(eRenderPass renderPass, SceneObject* sceneObject)
    {
        debug_assert(sceneObject);

        ObjectsCollection& renderPassList = mObjectsForRenderPass[renderPass];
        if (renderPassList.mElementsCount < MaxElements)
        {
            renderPassList.mElements[renderPassList.mElementsCount++] = sceneObject;
        }        
    }

    // discard all previously collected objects
    inline void Clear()
    {
        for (int icurr = 0; icurr < eRenderPass_Count; ++icurr)
        {
            mObjectsForRenderPass[icurr].mElementsCount = 0;
        }
    }

    // sort opaque objects by render program
    void SortOpaque();

    // sort translucent object by distance to camera
    void SortTranslucent();

public:
    // registered objects lists
    struct ObjectsCollection
    {
        int mElementsCount = 0;
        SceneObject* mElements[MaxElements];
    };

    ObjectsCollection mObjectsForRenderPass[eRenderPass_Count];
};
