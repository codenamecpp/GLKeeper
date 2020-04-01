#include "pch.h"
#include "GameObjectTransform.h"
#include "GameObject.h"
#include "SceneDefs.h"
#include "RenderScene.h"

GameObjectTransform::GameObjectTransform(GameObject* gameObject)
    : GameObjectComponent(eGameObjectComponent_Transform, gameObject)
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

void GameObjectTransform::ComputeTransformation()
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

void GameObjectTransform::SetLocalBoundingBox(const cxx::aabbox& aabox)
{
    mBounds = aabox;

    InvalidateBounds();
}

void GameObjectTransform::SetPosition(const glm::vec3& position)
{
    mPosition = position;

    InvalidateTransform();
}

void GameObjectTransform::SetScaling(float scaling)
{
    mScaling = scaling;

    InvalidateTransform();
}

void GameObjectTransform::SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward, const glm::vec3& directionUpward)
{
    mDirectionForward = directionForward;
    mDirectionRight = directionRight;
    mDirectionUpward = directionUpward;

    InvalidateTransform();
}

void GameObjectTransform::SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward)
{
    debug_assert(!"not yet implemented"); // todo
}

void GameObjectTransform::OrientTowards(const glm::vec3& point)
{
    OrientTowards(point, SceneAxisY);
}

void GameObjectTransform::Rotate(const glm::vec3& rotationAxis, float rotationAngle)
{
    glm::mat3 rotationMatrix = glm::mat3(glm::rotate(rotationAngle, rotationAxis)); 

    mDirectionForward = glm::normalize(rotationMatrix * mDirectionForward);
    mDirectionRight = glm::normalize(rotationMatrix * mDirectionRight);
    mDirectionUpward = glm::normalize(rotationMatrix * mDirectionUpward);

    InvalidateTransform();
}

void GameObjectTransform::Translate(const glm::vec3& translation)
{
    mPosition += translation;

    InvalidateTransform();
}

void GameObjectTransform::OrientTowards(const glm::vec3& point, const glm::vec3& upward)
{
    glm::vec3 zaxis = glm::normalize(point - mPosition);
    glm::vec3 xaxis = glm::normalize(glm::cross(upward, zaxis));
    glm::vec3 yaxis = glm::cross(zaxis, xaxis);

    // now assign orientation vectors
    SetOrientation(xaxis, zaxis, yaxis);
}

void GameObjectTransform::ResetTransformation()
{
    mTransformation = glm::mat4{1.0f};
    mScaling = 1.0f;
    mPosition = glm::vec3{0.0f};
    mDirectionRight = SceneAxisX;
    mDirectionUpward = SceneAxisY;
    mDirectionForward = SceneAxisZ;

    InvalidateTransform();
}

void GameObjectTransform::InvalidateTransform()
{
    if (mTransformDirty)
        return;

    mTransformDirty = true;
    mBoundingBoxDirty = true; // force refresh world space bounds 

    if (mGameObject->IsAttachedToScene())
    {
        gRenderScene.HandleTransformChange(mGameObject);
    }
}

void GameObjectTransform::InvalidateBounds()
{
    if (mBoundingBoxDirty)
        return;

    mBoundingBoxDirty = true;
    if (mGameObject->IsAttachedToScene())
    {
        gRenderScene.HandleTransformChange(mGameObject);
    }
}

void GameObjectTransform::ResetOrientation()
{
    SetOrientation(SceneAxisX, SceneAxisZ, SceneAxisY);
}