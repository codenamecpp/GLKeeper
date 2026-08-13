#include "stdafx.h"
#include "SceneObject.h"
#include "GameWorld.h"
#include "Scene.h"

SceneObject::SceneObject()
    : mRight(WorldAxes::X)
    , mUpward(WorldAxes::Y)
    , mForward(WorldAxes::Z)
    , mPosition(0.0f)
    , mScale(1.0f)
    , mTransformation(1.0f)
    , mBoundsTransformed()
    , mBounds()
    , mTransformDirty()
    , mBoundingBoxDirty()
    , mIsObjectActive()
    , mDebugColor(COLOR_GREEN)
{
    mBounds.set_to_zero();
}

SceneObject::~SceneObject()
{
    if (mIsObjectActive)
    {
        cxx_assert(mScene);
        mScene->OnAnimatorBecomeInactive(this);
    }
}

void SceneObject::SetScene(Scene* scene)
{
    mScene = scene;
}

void SceneObject::SetObjectActive(bool isActive)
{
    if (mIsObjectActive != isActive)
    {
        mIsObjectActive = isActive;

        cxx_assert(mScene);
        if (isActive)
        {
            mScene->OnAnimatorBecomeActive(this);
        }
        else
        {
            mScene->OnAnimatorBecomeInactive(this);
        }
    }
}

void SceneObject::SetPosition(const glm::vec3& position)
{
    if (mPosition != position)
    {
        mPosition = position;
        InvalidateTransform();
    }
}

void SceneObject::SetScale(float factor)
{
    const glm::vec3 scaleComponenets {factor};
    if (mScale != scaleComponenets)
    {
        mScale = scaleComponenets;
        InvalidateTransform();
    }
}

void SceneObject::SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward, const glm::vec3& directionUpward)
{
    mForward = directionForward;
    mRight = directionRight;
    mUpward = directionUpward;

    InvalidateTransform();
}

void SceneObject::SetOrientation(const glm::vec3& directionRight, const glm::vec3& directionForward)
{
    mForward = directionForward;
    mRight = directionRight;
    mUpward = WorldAxes::Y;

    InvalidateTransform();
}

void SceneObject::OrientTowards(const glm::vec3& point)
{
    mForward = glm::normalize(point - mPosition);
    mRight = glm::normalize(glm::cross(WorldAxes::Y, mForward));
    mUpward = glm::cross(mForward, mRight);

    InvalidateTransform();
}

void SceneObject::OrientTowards(const glm::vec3& point, const glm::vec3& upward)
{
    mForward = glm::normalize(point - mPosition);
    mRight = glm::normalize(glm::cross(upward, mForward));
    mUpward = glm::cross(mForward, mRight);

    InvalidateTransform();
}

void SceneObject::RotateAroundAxis(const glm::vec3& rotationAxis, cxx::angle_t rotationAngle)
{
    if (rotationAngle.mAngleRadians != 0.0f)
    {
        glm::mat3 rotationMatrix = glm::mat3(glm::rotate(rotationAngle.to_radians(), rotationAxis)); 

        mForward = glm::normalize(rotationMatrix * mForward);
        mRight = glm::normalize(rotationMatrix * mRight);
        mUpward = glm::normalize(rotationMatrix * mUpward);

        InvalidateTransform();
    }
}

void SceneObject::Translate(const glm::vec3& translation)
{
    static const glm::vec3 zeroTranslation {0.0f};
    if (translation != zeroTranslation)
    {
        mPosition += translation;
        InvalidateTransform();
    }
}

void SceneObject::SetLocalBoundingBox(const cxx::aabbox& bounds)
{
    if (bounds == mBounds) return;

    mBounds = bounds;
    InvalidateBounds();
}

void SceneObject::ResetOrientation()
{
    mForward = WorldAxes::Z;
    mRight = WorldAxes::X;
    mUpward = WorldAxes::Y;

    InvalidateTransform();
}

void SceneObject::ResetTransformation()
{
    mTransformation = glm::mat4{1.0f};
    mScale = glm::vec3(1.0f);
    mPosition = glm::vec3{0.0f};
    mForward = WorldAxes::Z;
    mRight = WorldAxes::X;
    mUpward = WorldAxes::Y;

    InvalidateTransform();
}

void SceneObject::InvalidateTransform()
{
    if (!mTransformDirty)
    {
        mTransformDirty = true;
        mBoundingBoxDirty = true; // force refresh world space bounds 

        if (mIsObjectActive)
        {
            cxx_assert(mScene);
            mScene->OnObjectInvalidateTransform(this);
        }
    }
}

void SceneObject::InvalidateBounds()
{
    if (!mBoundingBoxDirty)
    {
        mBoundingBoxDirty = true;

        if (mIsObjectActive)
        {
            cxx_assert(mScene);
            mScene->OnObjectInvalidateBounds(this);
        }
    }
}

void SceneObject::ComputeTransformation()
{
    // refresh transformations matrix
    if (mTransformDirty)
    {
        glm::mat4 orientation {1.0f};
        orientation[0] = glm::vec4(mRight, 0);
        orientation[1] = glm::vec4(mUpward, 0);
        orientation[2] = glm::vec4(mForward, 0);

        mTransformation = glm::translate(mPosition) * orientation * glm::scale(mScale);
        mTransformDirty = false;
        // force refresh world space bounds
        mBoundingBoxDirty = true;
    }

    // refresh world space bounding box
    if (mBoundingBoxDirty)
    {
        mBoundsTransformed = cxx::transform_aabbox(mBounds, mTransformation);
        mBoundingBoxDirty = false;
    }
}

void SceneObject::OnRecycle()
{
    SetObjectActive(false);
    ResetTransformation();

    mBoundsTransformed.set_to_zero();
    mBounds.set_to_zero();

    mRenderLayerMask = RenderLayer_WorldObjects;

    mScene = nullptr;
    mIsObjectActive = false;
}
