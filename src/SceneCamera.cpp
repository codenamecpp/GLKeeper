#include "pch.h"
#include "SceneCamera.h"

SceneCamera::SceneCamera()
    : mProjMatrixDirty(true)
    , mViewMatrixDirty(true)
    , mCurrentMode(eSceneCameraMode_Perspective)
{
    SetIdentity();
}

void SceneCamera::SetPerspective(float aspect, float fovy, float nearPlane, float farPlane)
{
    mCurrentMode = eSceneCameraMode_Perspective;
    mProjectionParams.mPerspective.mAspect = aspect;
    mProjectionParams.mPerspective.mFovy = fovy;
    mProjectionParams.mPerspective.mNearPlane = nearPlane;
    mProjectionParams.mPerspective.mFarPlane = farPlane;
    // force dirty flags
    mProjMatrixDirty = true;
}

void SceneCamera::SetOrthographic(float leftp, float rightp, float bottomp, float topp)
{
    mCurrentMode = eSceneCameraMode_Orthographic;
    mProjectionParams.mOrthographic.mLeftP = leftp;
    mProjectionParams.mOrthographic.mRightP = rightp;
    mProjectionParams.mOrthographic.mBottomP = bottomp;
    mProjectionParams.mOrthographic.mTopP = topp;
    // force dirty flags
    mProjMatrixDirty = true;
}

void SceneCamera::InvalidateTransformation()
{
    // force dirty flags
    mProjMatrixDirty = true;
    mViewMatrixDirty = true;
}

void SceneCamera::ComputeMatrices()
{
    bool computeViewProjectionMatrix = mProjMatrixDirty || mViewMatrixDirty;
    if (mProjMatrixDirty)
    {
        if (mCurrentMode == eSceneCameraMode_Perspective)
        {
            mProjectionMatrix = glm::perspective(glm::radians(mProjectionParams.mPerspective.mFovy), 
                mProjectionParams.mPerspective.mAspect, 
                mProjectionParams.mPerspective.mNearPlane, 
                mProjectionParams.mPerspective.mFarPlane);
        }
        else
        {
            mProjectionMatrix = glm::ortho(mProjectionParams.mOrthographic.mLeftP,
                mProjectionParams.mOrthographic.mRightP, 
                mProjectionParams.mOrthographic.mBottomP, 
                mProjectionParams.mOrthographic.mTopP);
        }
        mProjMatrixDirty = false;
    }

    if (mViewMatrixDirty)
    {
        mViewMatrix = glm::lookAt(mPosition, mPosition + mForwardVector, mUpVector);
        mViewMatrixDirty = false;
    }

    if (computeViewProjectionMatrix)
    {
        mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;

        // recompute frustum planes
        mFrustum.compute_from_viewproj_matrix(mViewProjectionMatrix);
    }
}

void SceneCamera::SetIdentity()
{
    // set ident matrices
    mProjectionMatrix = glm::mat4(1.0f);
    mViewProjectionMatrix = glm::mat4(1.0f);
    mViewMatrix = glm::mat4(1.0f);

    // set default axes
    mForwardVector = -SceneAxisZ; // look along negative axis
    mUpVector = SceneAxisY;
    mRightVector = SceneAxisX;

    // reset position to origin
    mPosition = glm::vec3(0.0f);

    mProjMatrixDirty = true;
    mViewMatrixDirty = true;
}

void SceneCamera::ResetOrientation()
{
    mForwardVector = -SceneAxisZ; // look along negative axis
    mUpVector = SceneAxisY;
    mRightVector = SceneAxisX;

    mViewMatrixDirty = true;
}

void SceneCamera::FocusAt(const glm::vec3& focusPoint)
{
    mForwardVector = glm::normalize(focusPoint - mPosition);

    // compute temporal up vector based on the forward vector
    // watch out when look up/down at 90 degree
    // for example, forward vector is on the Y axis
    if(fabs(mForwardVector.x) < FLT_EPSILON && fabs(mForwardVector.z) < FLT_EPSILON)
    {
        // forward vector is pointing +Y axis
        if (mForwardVector.y > 0)
        {
            mUpVector = -SceneAxisZ;
        }
        // forward vector is pointing -Y axis
        else
        {
            mUpVector = SceneAxisZ;
        }
    }
    // in general, up vector is straight up
    else
    {
        mUpVector = SceneAxisY;
    }

    mRightVector = glm::normalize(glm::cross(mUpVector, mForwardVector));
    mUpVector = glm::cross(mForwardVector, mRightVector);

    mViewMatrixDirty = true;
}

void SceneCamera::SetPosition(const glm::vec3& position)
{
    mPosition = position;

    mViewMatrixDirty = true;
}

void SceneCamera::SetRotationAngles(const glm::vec3& rotationAngles)
{
    const glm::mat4 rotationMatrix = glm::eulerAngleYXZ(
        glm::radians(rotationAngles.y), 
        glm::radians(rotationAngles.x), 
        glm::radians(rotationAngles.z));

    const glm::vec3 rotatedUp = glm::vec3(rotationMatrix * glm::vec4(SceneAxisY, 0.0f));
    mForwardVector = glm::vec3(rotationMatrix * glm::vec4(-SceneAxisZ, 0.0f));
    mRightVector = glm::normalize(glm::cross(rotatedUp, mForwardVector)); 
    mUpVector = glm::normalize(glm::cross(mForwardVector, mRightVector));

    mViewMatrixDirty = true;
}

void SceneCamera::Translate(const glm::vec3& direction)
{
    mPosition += direction;

    mViewMatrixDirty = true;
}

bool SceneCamera::CastRayFromScreenPoint(const glm::ivec2& screenCoordinate, const Rect2D& screenViewport, cxx::ray3d& resultRay)
{
    // wrap y
    const int32_t mouseY = screenViewport.mSizeY - screenCoordinate.y;
    const glm::ivec4 viewport { screenViewport.mX, screenViewport.mY, 
        screenViewport.mSizeX, screenViewport.mSizeY };
        //unproject twice to build a ray from near to far plane
    const glm::vec3 v0 = glm::unProject(glm::vec3{screenCoordinate.x * 1.0f, mouseY * 1.0f, 0.0f}, 
        mViewMatrix, 
        mProjectionMatrix, viewport); // near plane

    const glm::vec3 v1 = glm::unProject(glm::vec3{screenCoordinate.x * 1.0f, mouseY * 1.0f, 1.0f}, 
        mViewMatrix, 
        mProjectionMatrix, viewport); // far plane

    resultRay.mOrigin = v0;
    resultRay.mDirection = glm::normalize(v1 - v0);
    return true;
}

void SceneCamera::SetOrientation(const glm::vec3& dirForward, const glm::vec3& dirRight, const glm::vec3& dirUp)
{
    mForwardVector = dirForward;
    mUpVector = dirUp;
    mRightVector = dirRight;

    mViewMatrixDirty = true;
}
