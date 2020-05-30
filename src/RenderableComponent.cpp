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
    if (mMeshParts.empty())
        return;

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

int RenderableComponent::GetMeshPartsCount() const
{
    int numMeshParts = (int) mMeshParts.size();
    return numMeshParts;
}

MeshMaterial* RenderableComponent::GetMeshPartMaterial(int meshPartIndex)
{
    int numMeshParts = GetMeshPartsCount();
    if (numMeshParts < meshPartIndex)
    {
        int materialIndex = mMeshParts[meshPartIndex].mMaterialIndex;
        return GetMeshMaterial(materialIndex);
    }
    debug_assert(false);
    return nullptr;
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

void RenderableComponent::SetMeshPartsCount(int numMeshParts)
{
    mMeshParts.resize(numMeshParts);
}

void RenderableComponent::SetMeshPart(int meshPartIndex, int materialIndex, int vertexOffset, int indexOffset, int vertexCount, int triangleCount)
{
    int numMeshParts = GetMeshPartsCount();
    if (meshPartIndex < numMeshParts)
    {
        debug_assert(vertexCount > 0);
        // make sure that material exists
        int numMeshMaterials = (int) mMeshMaterials.size();
        if (materialIndex >= numMeshMaterials)
        {
            mMeshMaterials.resize(materialIndex + 1);
        }
        // copy mesh part data
        mMeshParts[meshPartIndex].mMaterialIndex = materialIndex;
        mMeshParts[meshPartIndex].mVertexDataOffset = vertexOffset;
        mMeshParts[meshPartIndex].mIndexDataOffset = indexOffset;
        mMeshParts[meshPartIndex].mVertexCount = vertexCount;
        mMeshParts[meshPartIndex].mTriangleCount = triangleCount;
        return;
    }
    debug_assert(false);
}

void RenderableComponent::ClearMeshParts()
{
    mMeshParts.clear();
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