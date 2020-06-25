#include "pch.h"
#include "SceneRenderList.h"
#include "SceneObject.h"

void SceneRenderList::SortOpaque()
{
    ObjectsCollection& renderPassList = mObjectsForRenderPass[eRenderPass_Opaque];

    if (renderPassList.mElementsCount > 2)
    {
        // dbg
        for (int iElement = 0; iElement < renderPassList.mElementsCount; ++iElement)
        {
            debug_assert(renderPassList.mElements[iElement]->mRenderProgram);
        }

        std::sort(renderPassList.mElements, renderPassList.mElements + renderPassList.mElementsCount, 
            [](const SceneObject* lhs, const SceneObject* rhs)
        {
            return lhs->mRenderProgram < rhs->mRenderProgram;
        });
    }
}

void SceneRenderList::SortTranslucent()
{
    ObjectsCollection& renderPassList = mObjectsForRenderPass[eRenderPass_Translucent];

    if (renderPassList.mElementsCount > 2)
    {
        std::sort(renderPassList.mElements, renderPassList.mElements + renderPassList.mElementsCount, 
            [](const SceneObject* lhs, const SceneObject* rhs)
        {
            return lhs->mDistanceToCameraSquared < rhs->mDistanceToCameraSquared;
        });
    }
}