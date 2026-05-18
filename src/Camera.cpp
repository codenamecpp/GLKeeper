#include "stdafx.h"
#include "Camera.h"

Camera::Camera()
    : mProjectionMatrix(1.0f)
    , mViewMatrix(1.0f)
    , mViewProjectionMatrix(1.0f)
    , mPosition()
    , mProjectionParams()
    , mProjMatrixDirty()
    , mViewMatrixDirty()
{
    ResetOrientation();
}

void Camera::ComputeMatricesAndFrustum(const Viewport& viewport)
{
    const Point2D viewportSize = viewport.GetAreaSize();

    bool viewportDirty = (mViewportSizeCache != viewportSize);
    if (viewportDirty)
    {
        mViewportSizeCache = viewportSize;
        mProjMatrixDirty = true; // force recompute projection matrix
    }

    bool computeViewProjectionMatrix = mProjMatrixDirty || mViewMatrixDirty;
    if (mProjMatrixDirty)
    {
        mProjectionMatrix = glm::perspective(glm::radians(mProjectionParams.mFovy), 
            viewport.GetAspectRatio(), 
            mProjectionParams.mNearDistance, 
            mProjectionParams.mFarDistance);
        mProjMatrixDirty = false;
    }

    if (mViewMatrixDirty)
    {
        mViewMatrix = glm::lookAt(mPosition, mPosition + mForward, mUp);
        mViewMatrixDirty = false;
    }

    if (computeViewProjectionMatrix)
    {
        mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;
        // recompute frustum planes
        mFrustum.compute_from_viewproj_matrix(mViewProjectionMatrix);
    }
}

void Camera::SetupProjection(const ProjectionParams& theDescription)
{
    mProjectionParams = theDescription;
    mProjMatrixDirty = true;
}

void Camera::SetPosition(const glm::vec3& position)
{
    mPosition = position;
    mViewMatrixDirty = true;
}

void Camera::SetRotation(const glm::vec3& rotationAngles)
{
    const glm::mat4 rotationMatrix = glm::eulerAngleYXZ(
        glm::radians(rotationAngles.y), 
        glm::radians(rotationAngles.x), 
        glm::radians(rotationAngles.z));

    const glm::vec3 rotatedUp = glm::vec3(rotationMatrix * glm::vec4(WorldAxes::Y, 0.0f));
    mForward = glm::vec3(rotationMatrix * glm::vec4(-WorldAxes::Z, 0.0f));
    mRight = glm::normalize(glm::cross(rotatedUp, mForward)); 
    mUp = glm::normalize(glm::cross(mForward, mRight));
    mViewMatrixDirty = true;
}

void Camera::ResetOrientation()
{
    mForward = -WorldAxes::Z; // look along negative axis
    mRight = WorldAxes::X;
    mUp = WorldAxes::Y;
    mViewMatrixDirty = true;
}

void Camera::LookAt(const glm::vec3& targetPosition, const glm::vec3& up)
{
    mForward = glm::normalize(targetPosition - mPosition);
    mRight = glm::normalize(glm::cross(up, mForward)); 
    mUp = glm::normalize(glm::cross(mForward, mRight));
    mViewMatrixDirty = true;
}

void Camera::Translate(const glm::vec3& direction)
{
    if (glm::length2(direction) > 0.0f)
    {
        mPosition += direction;
        mViewMatrixDirty = true;
    }
}