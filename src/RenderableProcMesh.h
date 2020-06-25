#pragma once

#include "SimpleTriangleMesh.h"
#include "VertexFormat.h"
#include "SceneObject.h"

// procedural static mesh scene object
class RenderableProcMesh: public SceneObject
{
    decl_rtti(RenderableProcMesh, SceneObject)

    friend class ProcMeshRenderer;

public:
    // mesh data, available for both read and write
    // after making modifications you need invalidate and update bounds manually
    std::vector<Vertex3D_TriMesh> mTriMeshParts;

public:
    RenderableProcMesh();

    // clear all triangle mesh parts and materials
    void ClearMesh();

    // compute bounding box for current mesh
    void UpdateBounds();

    // override RenderableObject methods
    void PrepareRenderResources() override;
    void ReleaseRenderResources() override;
    void RenderFrame(SceneRenderContext& renderContext) override;
};