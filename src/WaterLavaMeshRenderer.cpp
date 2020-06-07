#include "pch.h"
#include "WaterLavaMeshRenderer.h"
#include "RenderScene.h"
#include "GraphicsDevice.h"
#include "WaterLavaMeshComponent.h"
#include "cvars.h"
#include "GraphicsDevice.h"
#include "GpuBuffer.h"
#include "TerrainTile.h"

const int MaxWaterLavaMeshBufferSize = 1024 * 1024 * 2;

bool WaterLavaMeshRenderer::Initialize()
{
    if (!mWaterLavaRenderProgram.LoadProgram())
    {
        debug_assert(false);
    }
    return true;
}

void WaterLavaMeshRenderer::Deinit()
{
    mWaterLavaRenderProgram.FreeProgram();
}

void WaterLavaMeshRenderer::Render(SceneRenderContext& renderContext, WaterLavaMeshComponent* component)
{
    if (!gCVarRender_DrawWaterAndLava.mValue)
        return;

    debug_assert(component);

    if (component->mVertexBuffer == nullptr || component->mIndexBuffer == nullptr)
    {
        debug_assert(false);
        return;
    }

    mWaterLavaRenderProgram.SetViewProjectionMatrix(gRenderScene.mCamera.mViewProjectionMatrix);
    mWaterLavaRenderProgram.SetWaveParams(component->mWaveTime, component->mWaveWidth, component->mWaveHeight, component->mWaterlineHeight);
    mWaterLavaRenderProgram.ActivateProgram();

    // bind indices
    gGraphicsDevice.BindIndexBuffer(component->mIndexBuffer);
    gGraphicsDevice.BindVertexBuffer(component->mVertexBuffer, Vertex3D_WaterLava_Format::Get());

    for (WaterLavaMeshComponent::DrawCall& currPart: component->mDrawCalls)
    {
        if (currPart.mVertexCount == 0)
        {
            debug_assert(false);
            continue;
        }
        MeshMaterial* currMaterial = component->GetMeshMaterial(currPart.mMaterialIndex);
        if (currMaterial == nullptr)
        {
            debug_assert(false);
            continue;
        }
        currMaterial->ActivateMaterial();
        if (currPart.mTriangleCount)
        {
            gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32, 0, currPart.mTriangleCount * 3);
        }
        else
        {
            gGraphicsDevice.RenderPrimitives(ePrimitiveType_Triangles, 0, currPart.mVertexCount);
        }
    }    
}

void WaterLavaMeshRenderer::PrepareRenderdata(WaterLavaMeshComponent* component)
{
    debug_assert(component);
    if (component->mWaterLavaTiles.empty())
    {
        debug_assert(false);
        return;
    }

    const int NumVerticesPerTile = 9;
    const int NumTrianglesPerTile = 8;

    int vertexCount = component->mWaterLavaTiles.size() * NumVerticesPerTile;
    int triangleCount = component->mWaterLavaTiles.size() * NumTrianglesPerTile;

    component->SetDrawCallsCount(1);
    component->SetDrawCall(0, 0, 0, 0, vertexCount, triangleCount);

    int actualVBufferLength = vertexCount * Sizeof_Vertex3D_WaterLava;
    int actualIBufferLength = triangleCount * sizeof(glm::ivec3);
    if (actualVBufferLength > MaxWaterLavaMeshBufferSize || actualIBufferLength > MaxWaterLavaMeshBufferSize)
    {
        debug_assert(false);
        return;
    }

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
    if (!vertexBuffer->Setup(eBufferUsage_Static, actualVBufferLength, nullptr) ||
        !indexBuffer->Setup(eBufferUsage_Static, actualIBufferLength, nullptr))
    {
        debug_assert(false);
        return;
    }

    Vertex3D_WaterLava* vbufferPtr = vertexBuffer->LockData<Vertex3D_WaterLava>(BufferAccess_UnsynchronizedWrite, 0, actualVBufferLength);
    debug_assert(vbufferPtr);

    glm::ivec3* ibufferPtr = indexBuffer->LockData<glm::ivec3>(BufferAccess_UnsynchronizedWrite, 0, actualIBufferLength);
    debug_assert(ibufferPtr);

    // upload data
    int vertices_counter = 0;
    for (TerrainTile* currTile: component->mWaterLavaTiles)
    {
        // setup indices
        const glm::ivec3 pointindices[NumTrianglesPerTile] = {
            {3, 4, 0}, {4, 1, 0}, // 1
            {4, 2, 1}, {4, 5, 2}, // 2
            {6, 4, 3}, {6, 7, 4}, // 3
            {7, 8, 4}, {8, 5, 4}, // 4
        };

        for (const glm::ivec3& pointindex : pointindices)
        {
            ibufferPtr->x = vertices_counter + pointindex.x;
            ibufferPtr->y = vertices_counter + pointindex.y;
            ibufferPtr->z = vertices_counter + pointindex.z;
            ++ibufferPtr;
        }

        // setup vertices
        const glm::vec3 middlep = 
        {
            currTile->mTileLocation.x * 1.0f, 0.0f, 
            currTile->mTileLocation.y * 1.0f
        };

        const glm::vec3 positions[NumVerticesPerTile] = 
        {
            {middlep.x - TERRAIN_BLOCK_HALF_SIZE, middlep.y, middlep.z - TERRAIN_BLOCK_HALF_SIZE},
            {middlep.x,                          middlep.y, middlep.z - TERRAIN_BLOCK_HALF_SIZE},
            {middlep.x + TERRAIN_BLOCK_HALF_SIZE, middlep.y, middlep.z - TERRAIN_BLOCK_HALF_SIZE},
            {middlep.x - TERRAIN_BLOCK_HALF_SIZE, middlep.y, middlep.z},
            middlep,
            {middlep.x + TERRAIN_BLOCK_HALF_SIZE, middlep.y, middlep.z},
            {middlep.x - TERRAIN_BLOCK_HALF_SIZE, middlep.y, middlep.z + TERRAIN_BLOCK_HALF_SIZE},
            {middlep.x,                          middlep.y, middlep.z + TERRAIN_BLOCK_HALF_SIZE},
            {middlep.x + TERRAIN_BLOCK_HALF_SIZE, middlep.y, middlep.z + TERRAIN_BLOCK_HALF_SIZE},
        };

        const glm::vec2 tcoordsp = {
            currTile->mTileLocation.x * 1.0f,
            currTile->mTileLocation.y * 1.0f
        };

        const glm::vec2 texturecoords[NumVerticesPerTile] = {
            tcoordsp,
            {tcoordsp.x + 0.5f, tcoordsp.y},
            {tcoordsp.x + 1.0f, tcoordsp.y},
            {tcoordsp.x, tcoordsp.y + 0.5f},
            {tcoordsp.x + 0.5f, tcoordsp.y + 0.5f},
            {tcoordsp.x + 1.0, tcoordsp.y + 0.5f},
            {tcoordsp.x, tcoordsp.y + 1.0f},
            {tcoordsp.x + 0.5f, tcoordsp.y + 1.0f},
            {tcoordsp.x + 1.0f, tcoordsp.y + 1.0f},
        };

        // process vertices
        for (int ipoint = 0; ipoint < NumVerticesPerTile; ++ipoint)
        {
            vbufferPtr->mPosition = positions[ipoint];
            vbufferPtr->mTexcoord = texturecoords[ipoint];
            ++vbufferPtr;
        }

        vertices_counter += NumVerticesPerTile;
    }

    if (!indexBuffer->Unlock())
    {
        debug_assert(false);
    }

    if (!vertexBuffer->Unlock())
    {
        debug_assert(false);
    }
    
    component->mRenderProgram = &mWaterLavaRenderProgram;
}

void WaterLavaMeshRenderer::ReleaseRenderdata(WaterLavaMeshComponent* component)
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

    component->InvalidateMesh();
}