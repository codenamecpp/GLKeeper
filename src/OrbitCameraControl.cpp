#include "pch.h"
#include "OrbitCameraControl.h"
#include "InputsManager.h"
#include "SceneCamera.h"

OrbitCameraControl::OrbitCameraControl()
    : mPrevMousePositionX()
    , mPrevMousePositionY()
    , mFocusPoint()
    , mDegYaw()
    , mDegPitch()
    , mDistance(2.0f)
    , mDefaultYaw()
    , mDefaultPitch()
    , mDefaultDistance(2.0f)
{
}

void OrbitCameraControl::SetParams(float defaultYawDegrees, float defaultPitchDegrees, float defaultDistance)
{
    mDefaultDistance = defaultDistance;
    mDefaultPitch = defaultPitchDegrees;
    mDefaultYaw = defaultYawDegrees;
}

void OrbitCameraControl::ResetOrientation()
{
    mPrevMousePositionX = gInputsManager.mCursorPositionX;
    mPrevMousePositionY = gInputsManager.mCursorPositionY;
    mDegYaw = mDefaultYaw;
    mDegPitch = mDefaultPitch;
    mDistance = mDefaultDistance;
    mFocusPoint = glm::vec3(0.0f, 0.0f, 0.0f);

    UpdateOrientation();
}

void OrbitCameraControl::HandleUpdateFrame(float deltaTime)
{
}

void OrbitCameraControl::HandleSceneAttach()
{
}

void OrbitCameraControl::HandleSceneDetach()
{
}

void OrbitCameraControl::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (inputEvent.HasPressed(eMouseButton_Left) || inputEvent.HasPressed(eMouseButton_Right))
    {
        mPrevMousePositionX = gInputsManager.mCursorPositionX;
        mPrevMousePositionY = gInputsManager.mCursorPositionY;
    }
}

void OrbitCameraControl::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    debug_assert(mSceneCamera);

    bool orient = gInputsManager.GetMouseButtonL();
    bool moveorigin = gInputsManager.GetMouseButtonR();

    if (!orient && !moveorigin)
        return;

    int deltaX = inputEvent.mCursorPositionX - mPrevMousePositionX;
    int deltaY = inputEvent.mCursorPositionY - mPrevMousePositionY;

    mPrevMousePositionX = inputEvent.mCursorPositionX;
    mPrevMousePositionY = inputEvent.mCursorPositionY;

    const float speedUp = 0.75f;
    if (orient)
    {
        if (deltaY) // pitch
        {
            mDegPitch = cxx::normalize_angle_180(mDegPitch + (deltaY * speedUp));
            if (mDegPitch < 1.0f)
                mDegPitch = 1.0f;
            if (mDegPitch > 170.0f)
                mDegPitch = 170.0f;
        }
        if (deltaX) // yaw
        {
            mDegYaw = cxx::normalize_angle_180(mDegYaw + (deltaX * speedUp));
        }
    }

    if (moveorigin)
    {
        if (deltaX)
        {
            mFocusPoint += mSceneCamera->mRightDirection * (deltaX * -0.01f);
        }
        if (deltaY)
        {
            mFocusPoint += SceneAxis_Y() * (deltaY * -0.01f);
        }
    }

    if (deltaX || deltaY)
    {
        UpdateOrientation();
    }
}

void OrbitCameraControl::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    const float speedUp = 0.15f;

    mDistance -= inputEvent.mScrollY * speedUp;
    if (mDistance > 1000.0f)
        mDistance = 1000.0f;

    if (mDistance < 0.1f)
        mDistance = 0.1f;

    if (inputEvent.mScrollY)
    {
        UpdateOrientation();
    }
}

void OrbitCameraControl::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void OrbitCameraControl::HandleInputEvent(KeyCharEvent& inputEvent)
{
}

void OrbitCameraControl::UpdateOrientation()
{
    debug_assert(mSceneCamera);
    mSceneCamera->ResetOrientation();

    glm::vec3 campos;
    campos.x = glm::cos(glm::radians(mDegYaw)) * glm::sin(glm::radians(mDegPitch)) * mDistance;
    campos.y = glm::cos(glm::radians(mDegPitch)) * mDistance;
    campos.z = glm::sin(glm::radians(mDegYaw)) * glm::sin(glm::radians(mDegPitch)) * mDistance;

    campos += mFocusPoint;

    mSceneCamera->SetPosition(campos);
    mSceneCamera->FocusAt(mFocusPoint, SceneAxis_Y());
}