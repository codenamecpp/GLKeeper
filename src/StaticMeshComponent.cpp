#include "pch.h"
#include "StaticMeshComponent.h"
#include "RenderManager.h"
#include "GameObject.h"
#include "TransformComponent.h"

StaticMeshComponent::StaticMeshComponent(GameObject* gameObject)
    : RenderableComponent(gameObject)
{
}

void StaticMeshComponent::InvalidateMesh()
{
    if (mTriMeshParts.empty())
        return;

    mMeshInvalidated = true;
}

bool StaticMeshComponent::IsMeshInvalidated() const
{
    return mMeshInvalidated;
}

void StaticMeshComponent::ClearMesh()
{
    if (mTriMeshParts.empty())
        return;

    mTriMeshParts.clear();
    InvalidateMesh();

    ClearMeshMaterials();
}

void StaticMeshComponent::UpdateBounds()
{
    cxx::aabbox bounds;
    for (TriMeshPart& currMeshPart: mTriMeshParts)
    {
        currMeshPart.ComputeBounds();
        bounds.extend(currMeshPart.mBoundingBox);
    }

    mGameObject->mTransformComponent->SetLocalBoundingBox(bounds);
}

void StaticMeshComponent::RenderFrame(SceneRenderContext& renderContext)
{
    if (IsMeshInvalidated())
    {
        PrepareRenderResources();
    }

    StaticMeshComponentRenderer& renderer = gRenderManager.mStaticMeshRenderer;
    renderer.Render(renderContext, this);
}

void StaticMeshComponent::PrepareRenderResources()
{
    if (!IsMeshInvalidated())
        return;

    mMeshInvalidated = false;

    StaticMeshComponentRenderer& renderer = gRenderManager.mStaticMeshRenderer;
    renderer.PrepareRenderdata(this);
}

void StaticMeshComponent::ReleaseRenderResources()
{
    StaticMeshComponentRenderer& renderer = gRenderManager.mStaticMeshRenderer;
    renderer.ReleaseRenderdata(this);

    InvalidateMesh();
}