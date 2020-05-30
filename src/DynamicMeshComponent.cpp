#include "pch.h"
#include "DynamicMeshComponent.h"

DynamicMeshComponent::DynamicMeshComponent(GameObject* gameObject)
    : RenderableComponent(gameObject)
{
}

void DynamicMeshComponent::RenderFrame(SceneRenderContext& renderContext)
{
}