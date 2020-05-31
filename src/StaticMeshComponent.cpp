#include "pch.h"
#include "StaticMeshComponent.h"

StaticMeshComponent::StaticMeshComponent(GameObject* gameObject)
    : RenderableComponent(gameObject)
{
}

void StaticMeshComponent::RenderFrame(SceneRenderContext& renderContext)
{
}