#include "pch.h"
#include "RenderableComponent.h"
#include "SceneRenderList.h"

RenderableComponent::RenderableComponent(GameObject* sceneObject)
    : GameObjectComponent(sceneObject)
{
}

void RenderableComponent::PrepareRenderResources()
{
    // do nothing
}

void RenderableComponent::ReleaseRenderResources()
{
    // do nothing
}

void RenderableComponent::RenderFrame(SceneRenderContext& renderContext) 
{
    // do nothing
}

void RenderableComponent::RegisterForRendering(SceneRenderList& renderList)
{
    if (mDrawCalls.empty())
    {
        debug_assert(false);
        return;
    }

    bool hasOpaqueParts = false;
    bool hasTransparentParts = false;

    for (const MeshMaterial& currMaterial: mMeshMaterials)
    {
        if (!currMaterial.IsTransparent())
        {
            hasOpaqueParts = true;
        }
        else
        {
            hasTransparentParts = true;
        }
    }
    
    if (hasOpaqueParts)
    {
        renderList.RegisterRenderableComponent(eRenderPass_Opaque, this);
    }

    if (hasTransparentParts)
    {
        renderList.RegisterRenderableComponent(eRenderPass_Translucent, this);
    }
}

void RenderableComponent::DestroyComponent()
{
    ReleaseRenderResources();
    delete this;
}

int RenderableComponent::GetMaterialsCount() const
{
    int numMaterials = (int) mMeshMaterials.size();
    return numMaterials;
}

MeshMaterial* RenderableComponent::GetMeshMaterial(int materialIndex)
{
    int numMaterials = GetMaterialsCount();
    if (materialIndex < numMaterials)
        return &mMeshMaterials[materialIndex];

    debug_assert(false);
    return nullptr;
}

MeshMaterial* RenderableComponent::GetMeshMaterial() 
{
    return GetMeshMaterial(0);
}

void RenderableComponent::SetMeshMaterialsCount(int numMaterials)
{
    mMeshMaterials.resize(numMaterials);
}

void RenderableComponent::SetMeshMaterial(int materialIndex, const MeshMaterial& meshMaterial)
{
    int numMaterials = GetMaterialsCount();
    if (materialIndex < numMaterials)
    {
        mMeshMaterials[materialIndex] = meshMaterial;
        return;
    }
    debug_assert(false);
}

void RenderableComponent::ClearMeshMaterials()
{
    mMeshMaterials.clear();
}

void RenderableComponent::SetDrawCallsCount(int numMeshParts)
{
    mDrawCalls.resize(numMeshParts);
}

void RenderableComponent::SetDrawCall(int drawCall, int materialIndex, int vertexOffset, int indexOffset, int vertexCount, int triangleCount)
{
    int numDrawCalls = (int) mDrawCalls.size();
    if (drawCall < numDrawCalls)
    {
        debug_assert(vertexCount > 0);
        // make sure that material exists
        int numMaterials = (int) mMeshMaterials.size();
        if (materialIndex >= numMaterials)
        {
            mMeshMaterials.resize(materialIndex + 1);
        }
        // copy data
        mDrawCalls[drawCall].mMaterialIndex = materialIndex;
        mDrawCalls[drawCall].mVertexDataOffset = vertexOffset;
        mDrawCalls[drawCall].mIndexDataOffset = indexOffset;
        mDrawCalls[drawCall].mVertexCount = vertexCount;
        mDrawCalls[drawCall].mTriangleCount = triangleCount;
        return;
    }
    debug_assert(false);
}

void RenderableComponent::ClearDrawCalls()
{
    mDrawCalls.clear();
}

bool RenderableComponent::HasOpaqueMeshParts() const
{
    for (const MeshMaterial& currMaterial: mMeshMaterials)
    {
        if (!currMaterial.IsTransparent())
            return true;
    }
    return false;
}

bool RenderableComponent::HasTransparentMeshParts() const
{
    for (const MeshMaterial& currMaterial: mMeshMaterials)
    {
        if (currMaterial.IsTransparent())
            return true;
    }
    return false;
}