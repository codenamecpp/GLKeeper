#pragma once

#include "SceneObject.h"
#include "RenderMaterial.h"
#include "GraphicsDefs.h"

// terrain mesh instance
class TerrainMesh: public SceneObject
{
    friend class TerrainRenderer;

public:
    // readonly
    Rect2D mMapTerrainRect;

public:
    TerrainMesh();
    ~TerrainMesh();

    void SetTerrainArea(const Rect2D& mapArea);
    void InvalidateMesh();

    // rebuild terrain mesh and upload data to video memory
    void UpdateMesh();

    // request entity to register itself in render lists
    // @param renderList: Render lists
    void RegisterForRendering(SceneRenderList& renderList) override;

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