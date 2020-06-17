#pragma once

#include "SimpleTriangleMesh.h"
#include "VertexFormat.h"
#include "RenderableComponent.h"

// procedural static mesh component of entity
class ProcMeshComponent: public RenderableComponent
{
    decl_rtti(ProcMeshComponent, RenderableComponent)

    friend class ProcMeshRenderer;

public:
    // mesh data, available for both read and write
    // after making modifications you need invalidate and update bounds manually
    std::vector<Vertex3D_TriMesh> mTriMeshParts;

public:
    ProcMeshComponent(Entity* entity);

    // clear all triangle mesh parts and materials
    void ClearMesh();

    // compute bounding box for current mesh
    void UpdateBounds();

    // override RenderableComponent methods
    void PrepareRenderResources() override;
    void ReleaseRenderResources() override;
    void RenderFrame(SceneRenderContext& renderContext) override;
};