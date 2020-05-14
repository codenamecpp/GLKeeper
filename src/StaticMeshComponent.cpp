#include "pch.h"
#include "StaticMeshComponent.h"

StaticMeshComponent::StaticMeshComponent(GameObject* gameObject)
    : GameObjectComponent(eGameObjectComponent_StaticMesh, gameObject)
{
}

StaticMeshComponent::~StaticMeshComponent()
{
}

void StaticMeshComponent::RenderFrame(SceneRenderContext& renderContext)
{
}