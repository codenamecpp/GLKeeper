#pragma once

#include "GameObjectComponent.h"

// terrain mesh component of game object
class TerrainMeshComponent: public GameObjectComponent
{
    friend class TerrainMeshRenderer;
    friend class RenderManager;

public:
    // readonly
    Rectangle mMapTerrainRect;

public:
    TerrainMeshComponent(GameObject* gameObject);
    ~TerrainMeshComponent();

    // process render frame
    void RenderFrame(SceneRenderContext& renderContext) override;

    void SetTerrainArea(const Rectangle& mapArea);
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
        MeshMaterial mMaterial;

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

decl_gameobject_component(TerrainMeshComponent, eGameObjectComponent_TerrainMesh)