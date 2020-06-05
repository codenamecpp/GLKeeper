#include "pch.h"
#include "SceneRenderList.h"
#include "GameObject.h"
#include "RenderableComponent.h"

void SceneRenderList::SortOpaqueComponents()
{
    ComponentsList& renderPassList = mComponentsForRenderPass[eRenderPass_Opaque];

    if (renderPassList.mElementsCount > 2)
    {
        // dbg
        for (int iElement = 0; iElement < renderPassList.mElementsCount; ++iElement)
        {
            debug_assert(renderPassList.mElements[iElement]->mRenderProgram);
        }

        std::sort(renderPassList.mElements, renderPassList.mElements + renderPassList.mElementsCount, 
            [](const RenderableComponent* lhs, const RenderableComponent* rhs)
        {
            return lhs->mRenderProgram < rhs->mRenderProgram;
        });
    }
}

void SceneRenderList::SortTranslucentComponents()
{
    ComponentsList& renderPassList = mComponentsForRenderPass[eRenderPass_Translucent];

    if (renderPassList.mElementsCount > 2)
    {
        std::sort(renderPassList.mElements, renderPassList.mElements + renderPassList.mElementsCount, 
            [](const RenderableComponent* lhs, const RenderableComponent* rhs)
        {
            return lhs->mGameObject->mDistanceToCameraSquared < rhs->mGameObject->mDistanceToCameraSquared;
        });
    }
}