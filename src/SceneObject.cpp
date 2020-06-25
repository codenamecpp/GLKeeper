#include "pch.h"
#include "SceneObject.h"
#include "RenderScene.h"
#include "SceneRenderList.h"

SceneObject::SceneObject()
    : mDebugColor(Color32_Green)
    , mTransformDirty()
    , mBoundingBoxDirty()
    , mTransformation(1.0f)
    , mScaling(1.0f)
    , mPosition()
    , mDirectionRight(SceneAxisX)
    , mDirectionUpward(SceneAxisY)
    , mDirectionForward(SceneAxisZ)
{
    SetActive(true);
}

SceneObject::~SceneObject()
{
    SetActive(false);
}

void SceneObject::DestroyObject()
{
    ReleaseRenderResources();
    delete this;
}

void SceneObject::SetActive(bool isActive)
{
    if (isActive)
    {
        gRenderScene.AttachObject(this);
    }
    else
    {
        gRenderScene.DetachObject(this);
    }
}

bool SceneObject::IsActive() const
{
    return gRenderScene.IsObjectAttached(this);
}

void SceneObject::SetParentEntity(Entity* parentEntity)
{
    mParentEntity = parentEntity;
}

void SceneObject::ComputeTransformation()
{
    // refresh transformations matrix
    if (mTransformDirty)
    {
        glm::mat4 orientation {1.0f};
        orientation[0] = glm::vec4(mDirectionRight, 0);
        orientation[1] = glm::vec4(mDirectionUpward, 0);
        orientation[2] = glm::vec4(mDirectionForward, 0);

        glm::vec3 scaling {mScaling, mScaling, mScaling};
        mTransformation = glm::translate(mPosition) * orientation * glm::scale(scaling);
        mTransformDirty = false;
        // force refresh world space bounds
        mBoundsTransformed = cxx::transform_aabbox(mBounds, mTransformation);
        mBoundingBoxDirty = false;
        return;
    }

    // refresh world space bounding box
    if (mBoundingBoxDirty)
    {
        mBoundsTransformed = cxx::transform_aabbox(mBounds, mTransformation);
        mBoundingBoxDirty = false;
    }
}

void SceneObject::SetLocalBoundingBox(const cxx::aabbox& aabox)
{
    mBounds = aabox;

    InvalidateBounds();
}

void SceneObject::SetPosition(const glm::vec3& position)
{
    mPosition = position;

    InvalidateTransform();
}

void SceneObject::SetScaling(float scaling)
{
    mScaling = scaling;

    InvalidateTransform();
}

void SceneObject::SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward, const glm::vec3& directionUpward)
{
    mDirectionForward = directionForward;
    mDirectionRight = directionRight;
    mDirectionUpward = directionUpward;

    InvalidateTransform();
}

void SceneObject::SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward)
{
    debug_assert(!"not yet implemented"); // todo
}

void SceneObject::OrientTowards(const glm::vec3& point)
{
    OrientTowards(point, SceneAxisY);
}

void SceneObject::Rotate(const glm::vec3& rotationAxis, float rotationAngle)
{
    glm::mat3 rotationMatrix = glm::mat3(glm::rotate(rotationAngle, rotationAxis)); 

    mDirectionForward = glm::normalize(rotationMatrix * mDirectionForward);
    mDirectionRight = glm::normalize(rotationMatrix * mDirectionRight);
    mDirectionUpward = glm::normalize(rotationMatrix * mDirectionUpward);

    InvalidateTransform();
}

void SceneObject::Translate(const glm::vec3& translation)
{
    mPosition += translation;

    InvalidateTransform();
}

void SceneObject::OrientTowards(const glm::vec3& point, const glm::vec3& upward)
{
    glm::vec3 zaxis = glm::normalize(point - mPosition);
    glm::vec3 xaxis = glm::normalize(glm::cross(upward, zaxis));
    glm::vec3 yaxis = glm::cross(zaxis, xaxis);

    // now assign orientation vectors
    SetOrientation(xaxis, zaxis, yaxis);
}

void SceneObject::ResetTransformation()
{
    mTransformation = glm::mat4{1.0f};
    mScaling = 1.0f;
    mPosition = glm::vec3{0.0f};
    mDirectionRight = SceneAxisX;
    mDirectionUpward = SceneAxisY;
    mDirectionForward = SceneAxisZ;

    InvalidateTransform();
}

void SceneObject::InvalidateTransform()
{
    if (mTransformDirty)
        return;

    mTransformDirty = true;
    mBoundingBoxDirty = true; // force refresh world space bounds 
    gRenderScene.HandleTransformChange(this);
}

void SceneObject::InvalidateBounds()
{
    if (mBoundingBoxDirty)
        return;

    mBoundingBoxDirty = true;
    gRenderScene.HandleTransformChange(this);
}

void SceneObject::ResetOrientation()
{
    SetOrientation(SceneAxisX, SceneAxisZ, SceneAxisY);
}

void SceneObject::InvalidateMesh()
{
    ClearDrawParts();

    mMeshInvalidated = true;
}

bool SceneObject::IsMeshInvalidated() const
{
    return mMeshInvalidated;
}

void SceneObject::PrepareRenderResources()
{
    // do nothing
}

void SceneObject::ReleaseRenderResources()
{
    // do nothing
}

void SceneObject::RenderFrame(SceneRenderContext& renderContext) 
{
    // do nothing
}

void SceneObject::UpdateFrame(float deltaTime)
{
    // do nothing
}

void SceneObject::RegisterForRendering(SceneRenderList& renderList)
{
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
        renderList.RegisterSceneObject(eRenderPass_Opaque, this);
    }

    if (hasTransparentParts)
    {
        renderList.RegisterSceneObject(eRenderPass_Translucent, this);
    }
}

int SceneObject::GetMaterialsCount() const
{
    int numMaterials = (int) mMeshMaterials.size();
    return numMaterials;
}

const MeshMaterial* SceneObject::GetMeshMaterial(int materialIndex) const
{
    int numMaterials = GetMaterialsCount();
    if (materialIndex < numMaterials)
        return &mMeshMaterials[materialIndex];

    debug_assert(false);
    return nullptr;
}

const MeshMaterial* SceneObject::GetMeshMaterial() const
{
    return GetMeshMaterial(0);
}

MeshMaterial* SceneObject::GetMeshMaterial(int materialIndex)
{
    int numMaterials = GetMaterialsCount();
    if (materialIndex < numMaterials)
        return &mMeshMaterials[materialIndex];

    debug_assert(false);
    return nullptr;
}

MeshMaterial* SceneObject::GetMeshMaterial() 
{
    return GetMeshMaterial(0);
}

void SceneObject::SetMeshMaterialsCount(int numMaterials)
{
    mMeshMaterials.resize(numMaterials);
}

void SceneObject::SetMeshMaterial(int materialIndex, const MeshMaterial& meshMaterial)
{
    int numMaterials = GetMaterialsCount();
    if (materialIndex < numMaterials)
    {
        mMeshMaterials[materialIndex] = meshMaterial;
        return;
    }
    debug_assert(false);
}

void SceneObject::ClearMeshMaterials()
{
    mMeshMaterials.clear();
}

void SceneObject::SetDrawPartsCount(int numMeshParts)
{
    mDrawParts.resize(numMeshParts);
}

void SceneObject::SetDrawPart(int partsCount, int materialIndex, int vertexOffset, int indexOffset, int vertexCount, int triangleCount)
{
    int currentPartsCount = (int) mDrawParts.size();
    if (partsCount < currentPartsCount)
    {
        debug_assert(vertexCount > 0);
        // make sure that material exists
        int numMaterials = (int) mMeshMaterials.size();
        if (materialIndex >= numMaterials)
        {
            mMeshMaterials.resize(materialIndex + 1);
        }
        // copy data
        mDrawParts[partsCount].mMaterialIndex = materialIndex;
        mDrawParts[partsCount].mVertexDataOffset = vertexOffset;
        mDrawParts[partsCount].mIndexDataOffset = indexOffset;
        mDrawParts[partsCount].mVertexCount = vertexCount;
        mDrawParts[partsCount].mTriangleCount = triangleCount;
    }
    else
    {
        debug_assert(false);
    }
}

void SceneObject::ClearDrawParts()
{
    mDrawParts.clear();
}

bool SceneObject::HasOpaqueMeshParts() const
{
    for (const MeshMaterial& currMaterial: mMeshMaterials)
    {
        if (!currMaterial.IsTransparent())
            return true;
    }
    return false;
}

bool SceneObject::HasTransparentMeshParts() const
{
    for (const MeshMaterial& currMaterial: mMeshMaterials)
    {
        if (currMaterial.IsTransparent())
            return true;
    }
    return false;
}