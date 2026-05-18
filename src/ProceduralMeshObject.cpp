#include "stdafx.h"
#include "ProceduralMeshObject.h"

ProceduralMeshObject::ProceduralMeshObject(int vertsBufferCapacity, int trisBufferCapacity)
{
    mVertices.reserve(vertsBufferCapacity);
    mTriangles.reserve(trisBufferCapacity);
}

void ProceduralMeshObject::Clear()
{
    mVertices.clear();
    mTriangles.clear();

    InvalidateMesh();
}

void ProceduralMeshObject::InvalidateMesh()
{
    mMeshDataDirty = true;

    cxx::aabbox bounds;
    if (!mVertices.empty())
    {
        bounds.set_to_point(mVertices.front().mPosition);
        for (const Vertex3D& roller: mVertices)
        {
            bounds.extend(roller.mPosition);
        }
    }
    else
    {
        bounds.set_to_zero();
    }

    SetLocalBoundingBox(bounds);
}

void ProceduralMeshObject::UpdateFrame(float deltaTime)
{
}

void ProceduralMeshObject::RegisterForRendering(SceneRenderLists& renderLists, float distanceToCamera2)
{
    if (mVertices.empty()) return;

    eRenderPass renderPass = mRenderMaterial.IsOpaque() ? eRenderPass_Opaque : eRenderPass_Translucent;
    renderLists.Register(renderPass, this, distanceToCamera2);
}

void ProceduralMeshObject::OnRecycle()
{
    SceneObject::OnRecycle();

    mMeshDataDirty = false;
    mVertices.clear();
    mTriangles.clear();

    mGpuVertexBuffer.reset();
    mGpuIndexBuffer.reset();

    mUploadedTrisCount = 0;
    mUploadedVertsCount = 0;
}

void ProceduralMeshObject::PrepareRenderdata()
{
    if (!mMeshDataDirty)
        return;

    mMeshDataDirty = false;
    mUploadedVertsCount = 0;
    mUploadedTrisCount = 0;

    if (mVertices.empty())
        return;

    // allocate vertices
    const int vertexBufferLength = GetVertexCount() * Sizeof_Vertex3D;
    if (mGpuVertexBuffer == nullptr)
    {
        mGpuVertexBuffer = gRenderDevice.CreateVertexBuffer(eBufferUsage_Dynamic, vertexBufferLength);
        if ((mGpuVertexBuffer == nullptr) || !mGpuVertexBuffer->SubData(0, vertexBufferLength, mVertices.data()))
        {
            cxx_assert(false);
            return;
        }
        mGpuVertexBuffer->ConfigureVertexDefinitions(Vertex3D_Format::Get());
    }
    else if (!mGpuVertexBuffer->Create(eBufferUsage_Dynamic, vertexBufferLength, mVertices.data()))
    {
        cxx_assert(false);
        return;
    }
    mUploadedVertsCount = GetVertexCount();

    // allocate indices
    const int indexBufferLength = GetTriangleCount() * sizeof(glm::ivec3);
    if (indexBufferLength == 0)
        return;

    if (mGpuIndexBuffer == nullptr)
    {
        mGpuIndexBuffer = gRenderDevice.CreateIndexBuffer(eBufferUsage_Dynamic, indexBufferLength);
        if ((mGpuIndexBuffer == nullptr) || !mGpuIndexBuffer->SubData(0, indexBufferLength, mTriangles.data()))
        {
            cxx_assert(false);
            return;
        }
    }
    else if (!mGpuIndexBuffer->Create(eBufferUsage_Dynamic, indexBufferLength, mTriangles.data()))
    {
        cxx_assert(false);
        return;
    }
    mUploadedTrisCount = GetTriangleCount();
}
