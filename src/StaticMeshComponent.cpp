#include "pch.h"
#include "StaticMeshComponent.h"

StaticMeshComponent::StaticMeshComponent(GameObject* gameObject)
    : GameObjectComponent(GetComponentType(), gameObject)
{
}

StaticMeshComponent::~StaticMeshComponent()
{
}

void StaticMeshComponent::RenderFrame(SceneRenderContext& renderContext)
{
}