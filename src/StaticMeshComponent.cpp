#include "pch.h"
#include "StaticMeshComponent.h"
#include "RenderManager.h"
#include "Entity.h"
#include "TransformComponent.h"

StaticMeshComponent::StaticMeshComponent(Entity* entity)
    : RenderableComponent(entity)
{
}

void StaticMeshComponent::ClearMesh()
{
    if (mTriMeshParts.empty())
        return;

    mTriMeshParts.clear();
    InvalidateMesh();
}

void StaticMeshComponent::UpdateBounds()
{
    cxx::aabbox bounds;
    for (Vertex3D_TriMesh& currMeshPart: mTriMeshParts)
    {
        currMeshPart.ComputeBounds();
        bounds.extend(currMeshPart.mBoundingBox);
    }

    mParentEntity->mTransform->SetLocalBoundingBox(bounds);
}

void StaticMeshComponent::RenderFrame(SceneRenderContext& renderContext)
{
    StaticMeshRenderer& renderer = gRenderManager.mStaticMeshRenderer;
    renderer.Render(renderContext, this);
}

void StaticMeshComponent::PrepareRenderResources()
{
    if (!IsMeshInvalidated())
        return;

    StaticMeshRenderer& renderer = gRenderManager.mStaticMeshRenderer;
    renderer.PrepareRenderdata(this);
}

void StaticMeshComponent::ReleaseRenderResources()
{
    StaticMeshRenderer& renderer = gRenderManager.mStaticMeshRenderer;
    renderer.ReleaseRenderdata(this);

    InvalidateMesh();
}