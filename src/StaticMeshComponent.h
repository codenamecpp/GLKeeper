#pragma once

#include "SimpleTriangleMesh.h"
#include "VertexFormat.h"
#include "RenderableComponent.h"

// procedural static mesh component of game object
class StaticMeshComponent: public RenderableComponent
{
    decl_rtti(StaticMeshComponent, RenderableComponent)

    friend class StaticMeshRenderer;

public:
    // mesh data, available for both read and write
    // after making modifications you need invalidate and update bounds manually
    std::vector<Vertex3D_TriMesh> mTriMeshParts;

public:
    StaticMeshComponent(GameObject* gameObject);

    void InvalidateMesh();
    bool IsMeshInvalidated() const;

    // clear all triangle mesh parts and materials
    void ClearMesh();

    // compute bounding box for current mesh
    void UpdateBounds();

    // override RenderableComponent methods
    void PrepareRenderResources() override;
    void ReleaseRenderResources() override;
    void RenderFrame(SceneRenderContext& renderContext) override;

private:
    // dirty flag indicates that geometry is invalid and must be reuploaded
    bool mMeshInvalidated;
};