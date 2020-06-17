#include "pch.h"
#include "ProcMeshComponent.h"
#include "RenderManager.h"
#include "Entity.h"
#include "TransformComponent.h"

ProcMeshComponent::ProcMeshComponent(Entity* entity)
    : RenderableComponent(entity)
{
}

void ProcMeshComponent::ClearMesh()
{
    if (mTriMeshParts.empty())
        return;

    mTriMeshParts.clear();
    InvalidateMesh();
}

void ProcMeshComponent::UpdateBounds()
{
    cxx::aabbox bounds;
    for (Vertex3D_TriMesh& currMeshPart: mTriMeshParts)
    {
        currMeshPart.ComputeBounds();
        bounds.extend(currMeshPart.mBoundingBox);
    }

    mParentEntity->mTransform->SetLocalBoundingBox(bounds);
}

void ProcMeshComponent::RenderFrame(SceneRenderContext& renderContext)
{
    ProcMeshRenderer& renderer = gRenderManager.mProcMeshRenderer;
    renderer.Render(renderContext, this);
}

void ProcMeshComponent::PrepareRenderResources()
{
    if (!IsMeshInvalidated())
        return;

    ProcMeshRenderer& renderer = gRenderManager.mProcMeshRenderer;
    renderer.PrepareRenderdata(this);
}

void ProcMeshComponent::ReleaseRenderResources()
{
    ProcMeshRenderer& renderer = gRenderManager.mProcMeshRenderer;
    renderer.ReleaseRenderdata(this);

    InvalidateMesh();
}