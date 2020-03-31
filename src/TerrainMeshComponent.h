#pragma once

#include "SceneObjectComponent.h"
#include "RenderMaterial.h"

// terrain mesh component of scene object
class TerrainMeshComponent: public SceneObjectComponent
{
    friend class TerrainMeshRenderer;
    friend class RenderManager;

public:
    // readonly
    Rect2D mMapTerrainRect;

public:
    TerrainMeshComponent(SceneObject* sceneObject);
    ~TerrainMeshComponent();

    // process render frame
    void RenderFrame(SceneRenderContext& renderContext) override;

    void SetTerrainArea(const Rect2D& mapArea);
    void InvalidateMesh();

    // rebuild terrain mesh and upload data to video memory
    void UpdateMesh();

    // test whether mesh data is invalid
    bool IsMeshInvalidated() const;

private:
    void PrepareRenderData();
    void DestroyRenderData();

private:

    struct MeshBatch
    {
    public:
        RenderMaterial mMaterial;

        int mVertexStart;
        int mVertexCount;

        int mTriangleStart;
        int mTriangleCount;
    };

    // render data
    GpuBuffer* mVerticesBuffer = nullptr;
    GpuBuffer* mIndicesBuffer = nullptr;
    std::vector<MeshBatch> mBatchArray;

    bool mMeshDirty; // dirty flag indicates that geometry is invalid and must be reuploaded
};