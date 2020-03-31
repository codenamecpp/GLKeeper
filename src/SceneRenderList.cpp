#include "pch.h"
#include "SceneRenderList.h"
#include "SceneObject.h"

void SceneRenderList::SortOpaqueComponents()
{
    ComponentsList& renderPassList = mComponentsForRenderPass[eRenderPass_Opaque];

    if (renderPassList.mElementsCount > 2)
    {
        std::sort(renderPassList.mElements, renderPassList.mElements + renderPassList.mElementsCount, 
            [](const SceneObjectComponent* lhs, const SceneObjectComponent* rhs)
        {
            return lhs->mComponentType < rhs->mComponentType;
        });
    }
}

void SceneRenderList::SortTranslucentComponents()
{
    ComponentsList& renderPassList = mComponentsForRenderPass[eRenderPass_Translucent];

    if (renderPassList.mElementsCount > 2)
    {
        std::sort(renderPassList.mElements, renderPassList.mElements + renderPassList.mElementsCount, 
            [](const SceneObjectComponent* lhs, const SceneObjectComponent* rhs)
        {
            return lhs->mSceneObject->mDistanceToCameraSquared < rhs->mSceneObject->mDistanceToCameraSquared;
        });
    }
}