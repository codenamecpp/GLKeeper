#include "pch.h"
#include "SceneObject.h"
#include "GameScene.h"

SceneObject::SceneObject()
    : mListNodeTransformed(this)
    , mListNodeAttached(this)
    , mTransformationDirty()
    , mBoundingBoxDirty()
    , mTransformation(1.0f)
    , mScaling(1.0f)
    , mPosition()
    , mDirectionRight(SceneAxis_X())
    , mDirectionUpward(SceneAxis_Y())
    , mDirectionForward(SceneAxis_Z())
{
}

SceneObject::~SceneObject()
{
    // entity must be detached from scene before destroy
    debug_assert(IsAttachedToScene() == false);
}

void SceneObject::ComputeTransformation()
{
    // refresh transformations matrix
    if (mTransformationDirty)
    {
        glm::mat4 orientation {1.0f};
        orientation[0] = glm::vec4(mDirectionRight, 0);
        orientation[1] = glm::vec4(mDirectionUpward, 0);
        orientation[2] = glm::vec4(mDirectionForward, 0);

        glm::vec3 scaling {mScaling, mScaling, mScaling};
        mTransformation = glm::translate(mPosition) * orientation * glm::scale(scaling);
        mTransformationDirty = false;
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
    OrientTowards(point, SceneAxis_Y());
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
    mDirectionRight = SceneAxis_X();
    mDirectionUpward = SceneAxis_Y();
    mDirectionForward = SceneAxis_Z();

    InvalidateTransform();
}

void SceneObject::InvalidateTransform()
{
    if (mTransformationDirty)
        return;

    mTransformationDirty = true;
    mBoundingBoxDirty = true; // force refresh world space bounds 

    if (IsAttachedToScene())
    {
        gGameScene.HandleSceneObjectTransformChange(this);
    }
}

void SceneObject::InvalidateBounds()
{
    if (mBoundingBoxDirty)
        return;

    mBoundingBoxDirty = true;
    if (IsAttachedToScene())
    {
        gGameScene.HandleSceneObjectTransformChange(this);
    }
}

void SceneObject::ResetOrientation()
{
    SetOrientation(SceneAxis_X(), SceneAxis_Z(), SceneAxis_Y());
}

bool SceneObject::IsAttachedToScene() const
{
    return mListNodeAttached.is_linked();
}

