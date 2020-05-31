#include "pch.h"
#include "StaticMeshComponentRenderer.h"
#include "GpuBuffer.h"
#include "StaticMeshComponent.h"
#include "GraphicsDevice.h"

bool StaticMeshComponentRenderer::Initialize()
{
    if (!mStaticMeshRenderProgram.LoadProgram())
    {
        debug_assert(false);
    }

    return true;
}

void StaticMeshComponentRenderer::Deinit()
{
    mStaticMeshRenderProgram.FreeProgram();
}

void StaticMeshComponentRenderer::Render(SceneRenderContext& renderContext, StaticMeshComponent* component)
{
    debug_assert(component);
}

void StaticMeshComponentRenderer::PrepareRenderdata(StaticMeshComponent* component)
{
    debug_assert(component);
    if (component->mTriMeshParts.empty())
        return;

    component->ClearDrawCalls();

    int overallVertexCount = 0;
    int overallTriangleCount = 0;
    for (StaticMeshComponent::TriMeshPart& currPart: component->mTriMeshParts)
    {
        overallVertexCount += (int) currPart.mVertices.size();
        overallTriangleCount += (int) currPart.mTriangles.size();
    }

    if (overallVertexCount == 0)
        return;

    int actualVBufferLength = overallVertexCount * Sizeof_Vertex3D;
    int actualIBufferLength = overallTriangleCount * sizeof(glm::ivec3);

    // allocate vertex buffer object
    if (component->mVertexBuffer == nullptr)
    {
        component->mVertexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices);
        if (component->mVertexBuffer == nullptr)
        {
            debug_assert(false);
            return;
        }
    }

    // allocate index buffer object
    if (component->mIndexBuffer == nullptr)
    {
        component->mIndexBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Indices);
        if (component->mIndexBuffer == nullptr)
        {
            debug_assert(false);
            return;
        }
    }

    GpuBuffer* vertexBuffer = component->mVertexBuffer;
    GpuBuffer* indexBuffer = component->mIndexBuffer;

    // setup buffers
    if (!vertexBuffer->Setup(eBufferUsage_Dynamic, actualVBufferLength, nullptr) ||
        !indexBuffer->Setup(eBufferUsage_Dynamic, actualIBufferLength, nullptr))
    {
        debug_assert(false);
        return;
    }

    int numPieces = (int) component->mTriMeshParts.size();
    component->SetDrawCallsCount(numPieces);

    // upload parts
    unsigned char* vbufferPtr = vertexBuffer->LockData<unsigned char>(BufferAccess_UnsynchronizedWrite, 0, actualVBufferLength);
    debug_assert(vbufferPtr);

    unsigned char* ibufferPtr = indexBuffer->LockData<unsigned char>(BufferAccess_UnsynchronizedWrite, 0, actualIBufferLength);
    debug_assert(ibufferPtr);

    unsigned int iCurentPart = 0;
    unsigned int vertexDataOffset = 0;
    unsigned int triangleDataOffset = 0;

    for (StaticMeshComponent::TriMeshPart& currPart: component->mTriMeshParts)
    {
        RenderableComponent::DrawCall& drawCall = component->mDrawCalls[iCurentPart];
        drawCall.mMaterialIndex = iCurentPart;
        drawCall.mTriangleCount = (int) currPart.mTriangles.size();
        drawCall.mVertexCount = (int) currPart.mVertices.size();
        drawCall.mIndexDataOffset = triangleDataOffset;
        drawCall.mVertexDataOffset = vertexDataOffset;

        // copy geometry data
        unsigned int vertexDataLength = drawCall.mVertexCount * Sizeof_Vertex3D;
        unsigned int triangleDataLength = drawCall.mTriangleCount * sizeof(glm::ivec3);
        ::memcpy(vbufferPtr + vertexDataOffset, currPart.mVertices.data(), vertexDataLength);
        ::memcpy(ibufferPtr + triangleDataOffset, currPart.mTriangles.data(), triangleDataLength);

        vertexDataOffset += vertexDataLength;
        triangleDataOffset += triangleDataLength;
        ++iCurentPart;
    }

    if (!indexBuffer->Unlock())
    {
        debug_assert(false);
    }

    if (!vertexBuffer->Unlock())
    {
        debug_assert(false);
    }

    component->mRenderProgram = &mStaticMeshRenderProgram;
}

void StaticMeshComponentRenderer::ReleaseRenderdata(StaticMeshComponent* component)
{
    debug_assert(component);
    component->mRenderProgram = nullptr;
    if (component->mVertexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(component->mVertexBuffer);
        component->mVertexBuffer = nullptr;
    }

    if (component->mIndexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(component->mIndexBuffer);
        component->mIndexBuffer = nullptr;
    }
    component->ClearDrawCalls();
}