#include "pch.h"
#include "TerrainMeshRenderer.h"
#include "TerrainMesh.h"
#include "RenderScene.h"
#include "GraphicsDevice.h"

bool TerrainMeshRenderer::Initialize()
{
    if (!mTerrainRenderProgram.LoadProgram())
    {
        debug_assert(false);
    }
    return true;
}

void TerrainMeshRenderer::Deinit()
{
    mTerrainRenderProgram.FreeProgram();
}

void TerrainMeshRenderer::RenderTerrainMesh(SceneRenderContext& renderContext, TerrainMesh* terrainMesh)
{
    if (terrainMesh == nullptr || terrainMesh->mBatchArray.empty())
    {
        debug_assert(false);
        return;
    }

    mTerrainRenderProgram.SetViewProjectionMatrix(gRenderScene.mCamera.mViewProjectionMatrix);
    mTerrainRenderProgram.SetModelMatrix(terrainMesh->mTransformation);
    mTerrainRenderProgram.ActivateProgram();

    // bind indices
    gGraphicsDevice.BindIndexBuffer(terrainMesh->mIndicesBuffer);
    gGraphicsDevice.BindVertexBuffer(terrainMesh->mVerticesBuffer, Vertex3D_Terrain_Format::Get());

    for (TerrainMesh::MeshBatch& currBatch: terrainMesh->mBatchArray)
    {
        currBatch.mMaterial.ActivateMaterial();

        // filter out submeshes depending on current render pass
        if (renderContext.mCurrentPass == eRenderPass_Translucent && !currBatch.mMaterial.IsTransparent())
            continue;

        if (renderContext.mCurrentPass == eRenderPass_Opaque && currBatch.mMaterial.IsTransparent())
            continue;

        currBatch.mMaterial.ActivateMaterial();

        gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32,
            currBatch.mTriangleStart * sizeof(glm::ivec3), 
            currBatch.mTriangleCount * 3);
    }
}