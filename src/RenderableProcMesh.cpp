#include "pch.h"
#include "RenderableProcMesh.h"
#include "RenderManager.h"
#include "SceneObject.h"

RenderableProcMesh::RenderableProcMesh()
{
}

void RenderableProcMesh::ClearMesh()
{
    if (mTriMeshParts.empty())
        return;

    mTriMeshParts.clear();
    InvalidateMesh();
}

void RenderableProcMesh::UpdateBounds()
{
    cxx::aabbox bounds;
    for (Vertex3D_TriMesh& currMeshPart: mTriMeshParts)
    {
        currMeshPart.ComputeBounds();
        bounds.extend(currMeshPart.mBoundingBox);
    }

    SetLocalBoundingBox(bounds);
}

void RenderableProcMesh::RenderFrame(SceneRenderContext& renderContext)
{
    ProcMeshRenderer& renderer = gRenderManager.mProcMeshRenderer;
    renderer.Render(renderContext, this);
}

void RenderableProcMesh::PrepareRenderResources()
{
    if (!IsMeshInvalidated())
        return;

    ProcMeshRenderer& renderer = gRenderManager.mProcMeshRenderer;
    renderer.PrepareRenderdata(this);
}

void RenderableProcMesh::ReleaseRenderResources()
{
    ProcMeshRenderer& renderer = gRenderManager.mProcMeshRenderer;
    renderer.ReleaseRenderdata(this);

    InvalidateMesh();
}