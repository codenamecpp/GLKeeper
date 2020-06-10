#include "pch.h"
#include "RenderableComponent.h"
#include "SceneRenderList.h"

RenderableComponent::RenderableComponent(Entity* sceneObject)
    : EntityComponent(sceneObject)
{
}

void RenderableComponent::InvalidateMesh()
{
    ClearDrawCalls();

    mMeshInvalidated = true;
}

bool RenderableComponent::IsMeshInvalidated() const
{
    return mMeshInvalidated;
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
    if (!IsComponentActive())
        return;

    bool hasOpaqueParts = false;
    bool hasTransparentParts = false;

    // force regenerate renderdata
    if (IsMeshInvalidated())
    {
        PrepareRenderResources();

        mMeshInvalidated = false;
    }

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

void RenderableComponent::AwakeComponent()
{
}

void RenderableComponent::UpdateComponent(float deltaTime)
{
    // do nothing
}

void RenderableComponent::DeleteComponent()
{
    ReleaseRenderResources();
    delete this;
}

int RenderableComponent::GetMaterialsCount() const
{
    int numMaterials = (int) mMeshMaterials.size();
    return numMaterials;
}

const MeshMaterial* RenderableComponent::GetMeshMaterial(int materialIndex) const
{
    int numMaterials = GetMaterialsCount();
    if (materialIndex < numMaterials)
        return &mMeshMaterials[materialIndex];

    debug_assert(false);
    return nullptr;
}

const MeshMaterial* RenderableComponent::GetMeshMaterial() const
{
    return GetMeshMaterial(0);
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