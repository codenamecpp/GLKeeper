#include "pch.h"
#include "SceneRenderList.h"
#include "GameObject.h"

void SceneRenderList::SortOpaqueComponents()
{
    ComponentsList& renderPassList = mComponentsForRenderPass[eRenderPass_Opaque];

    if (renderPassList.mElementsCount > 2)
    {
        std::sort(renderPassList.mElements, renderPassList.mElements + renderPassList.mElementsCount, 
            [](const GameObjectComponent* lhs, const GameObjectComponent* rhs)
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
            [](const GameObjectComponent* lhs, const GameObjectComponent* rhs)
        {
            return lhs->mGameObject->mDistanceToCameraSquared < rhs->mGameObject->mDistanceToCameraSquared;
        });
    }
}