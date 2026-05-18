#pragma once

//////////////////////////////////////////////////////////////////////////

#include "SceneObject.h"

//////////////////////////////////////////////////////////////////////////

class ProceduralMeshObject: public SceneObject
{
    friend class ProceduralMeshRenderer;

public:
    ProceduralMeshObject(int vertsBufferCapacity = 64, int trisBufferCapacity = 64);

    void Clear();

    // accessing render material for reading or writing
    inline       auto& GetMaterial()       { return mRenderMaterial; }
    inline const auto& GetMaterial() const { return mRenderMaterial; }

    // accessing vertex buffer for reading or writing
    inline       auto& GetVertices()       { return mVertices; }
    inline const auto& GetVertices() const { return mVertices; }

    // accessing triangle buffer for reading or writing
    inline       auto& GetTriangles()       { return mTriangles; }
    inline const auto& GetTriangles() const { return mTriangles; }
    
    // request re-upload geometry to gpu after modify
    void InvalidateMesh();

    // elements count
    inline int GetVertexCount()   const { return static_cast<int>(mVertices.size());  }
    inline int GetTriangleCount() const { return static_cast<int>(mTriangles.size()); }

    // override SceneObject
    void UpdateFrame(float deltaTime) override;
    void RegisterForRendering(SceneRenderLists& renderLists, float distanceToCamera2) override;

    // pool
    void OnRecycle() override;

private:
    void PrepareRenderdata();

private:
    SurfaceMaterial mRenderMaterial;
    std::vector<Vertex3D> mVertices;
    std::vector<glm::ivec3> mTriangles;

    // render data
    std::unique_ptr<GpuVertexBuffer> mGpuVertexBuffer;
    std::unique_ptr<GpuIndexBuffer> mGpuIndexBuffer;
    int mUploadedTrisCount = 0;
    int mUploadedVertsCount = 0;
    bool mMeshDataDirty = false;
};