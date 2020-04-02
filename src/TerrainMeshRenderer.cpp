#include "pch.h"
#include "TerrainMeshRenderer.h"
#include "RenderScene.h"
#include "GraphicsDevice.h"
#include "TerrainMeshComponent.h"
#include "cvars.h"

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

void TerrainMeshRenderer::Render(SceneRenderContext& renderContext, TerrainMeshComponent* component)
{
    if (!gCVarRender_DrawTerrain.mValue)
        return;

    if (component == nullptr || component->mBatchArray.empty())
    {
        debug_assert(false);
        return;
    }

    static glm::mat4 identyMatrix {1.0f};

    mTerrainRenderProgram.SetViewProjectionMatrix(gRenderScene.mCamera.mViewProjectionMatrix);
    mTerrainRenderProgram.SetModelMatrix(SceneIdentyMatrix);
    mTerrainRenderProgram.ActivateProgram();

    // bind indices
    gGraphicsDevice.BindIndexBuffer(component->mIndicesBuffer);
    gGraphicsDevice.BindVertexBuffer(component->mVerticesBuffer, Vertex3D_Terrain_Format::Get());

    for (TerrainMeshComponent::MeshBatch& currBatch: component->mBatchArray)
    {
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