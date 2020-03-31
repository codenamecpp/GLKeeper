#include "pch.h"
#include "SceneObjectTransform.h"
#include "SceneObject.h"
#include "RenderScene.h"

SceneObjectTransform::SceneObjectTransform(SceneObject* sceneObject)
    : SceneObjectComponent(eSceneObjectComponent_Transform, sceneObject)
    , mTransformDirty()
    , mBoundingBoxDirty()
    , mTransformation(1.0f)
    , mScaling(1.0f)
    , mPosition()
    , mDirectionRight(SceneAxisX)
    , mDirectionUpward(SceneAxisY)
    , mDirectionForward(SceneAxisZ)
{
}

void SceneObjectTransform::ComputeTransformation()
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

void SceneObjectTransform::SetLocalBoundingBox(const cxx::aabbox& aabox)
{
    mBounds = aabox;

    InvalidateBounds();
}

void SceneObjectTransform::SetPosition(const glm::vec3& position)
{
    mPosition = position;

    InvalidateTransform();
}

void SceneObjectTransform::SetScaling(float scaling)
{
    mScaling = scaling;

    InvalidateTransform();
}

void SceneObjectTransform::SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward, const glm::vec3& directionUpward)
{
    mDirectionForward = directionForward;
    mDirectionRight = directionRight;
    mDirectionUpward = directionUpward;

    InvalidateTransform();
}

void SceneObjectTransform::SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward)
{
    debug_assert(!"not yet implemented"); // todo
}

void SceneObjectTransform::OrientTowards(const glm::vec3& point)
{
    OrientTowards(point, SceneAxisY);
}

void SceneObjectTransform::Rotate(const glm::vec3& rotationAxis, float rotationAngle)
{
    glm::mat3 rotationMatrix = glm::mat3(glm::rotate(rotationAngle, rotationAxis)); 

    mDirectionForward = glm::normalize(rotationMatrix * mDirectionForward);
    mDirectionRight = glm::normalize(rotationMatrix * mDirectionRight);
    mDirectionUpward = glm::normalize(rotationMatrix * mDirectionUpward);

    InvalidateTransform();
}

void SceneObjectTransform::Translate(const glm::vec3& translation)
{
    mPosition += translation;

    InvalidateTransform();
}

void SceneObjectTransform::OrientTowards(const glm::vec3& point, const glm::vec3& upward)
{
    glm::vec3 zaxis = glm::normalize(point - mPosition);
    glm::vec3 xaxis = glm::normalize(glm::cross(upward, zaxis));
    glm::vec3 yaxis = glm::cross(zaxis, xaxis);

    // now assign orientation vectors
    SetOrientation(xaxis, zaxis, yaxis);
}

void SceneObjectTransform::ResetTransformation()
{
    mTransformation = glm::mat4{1.0f};
    mScaling = 1.0f;
    mPosition = glm::vec3{0.0f};
    mDirectionRight = SceneAxisX;
    mDirectionUpward = SceneAxisY;
    mDirectionForward = SceneAxisZ;

    InvalidateTransform();
}

void SceneObjectTransform::InvalidateTransform()
{
    if (mTransformDirty)
        return;

    mTransformDirty = true;
    mBoundingBoxDirty = true; // force refresh world space bounds 

    if (mSceneObject->IsAttachedToScene())
    {
        gRenderScene.HandleTransformChange(mSceneObject);
    }
}

void SceneObjectTransform::InvalidateBounds()
{
    if (mBoundingBoxDirty)
        return;

    mBoundingBoxDirty = true;
    if (mSceneObject->IsAttachedToScene())
    {
        gRenderScene.HandleTransformChange(mSceneObject);
    }
}

void SceneObjectTransform::ResetOrientation()
{
    SetOrientation(SceneAxisX, SceneAxisZ, SceneAxisY);
}