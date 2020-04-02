#include "pch.h"
#include "OrbitCameraController.h"
#include "InputsManager.h"
#include "SceneCamera.h"

OrbitCameraController::OrbitCameraController()
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

void OrbitCameraController::SetParams(float defaultYawDegrees, float defaultPitchDegrees, float defaultDistance)
{
    mDefaultDistance = defaultDistance;
    mDefaultPitch = defaultPitchDegrees;
    mDefaultYaw = defaultYawDegrees;
}

void OrbitCameraController::ResetOrientation()
{
    mPrevMousePositionX = gInputsManager.mCursorPositionX;
    mPrevMousePositionY = gInputsManager.mCursorPositionY;
    mDegYaw = mDefaultYaw;
    mDegPitch = mDefaultPitch;
    mDistance = mDefaultDistance;
    mFocusPoint = glm::vec3(0.0f, 0.0f, 0.0f);

    SetupCameraView();
}

void OrbitCameraController::HandleUpdateFrame(float deltaTime)
{
}

void OrbitCameraController::HandleSceneAttach()
{
}

void OrbitCameraController::HandleSceneDetach()
{
}

void OrbitCameraController::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (inputEvent.HasPressed(eMouseButton_Left) || inputEvent.HasPressed(eMouseButton_Right))
    {
        mPrevMousePositionX = gInputsManager.mCursorPositionX;
        mPrevMousePositionY = gInputsManager.mCursorPositionY;
    }
}

void OrbitCameraController::HandleInputEvent(MouseMovedInputEvent& inputEvent)
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
            mFocusPoint += mSceneCamera->mRightVector * (deltaX * -0.01f);
        }
        if (deltaY)
        {
            mFocusPoint += SceneAxisY * (deltaY * -0.01f);
        }
    }

    if (deltaX || deltaY)
    {
        SetupCameraView();
    }
}

void OrbitCameraController::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    const float speedUp = 0.15f;

    mDistance -= inputEvent.mScrollY * speedUp;
    if (mDistance > 1000.0f)
        mDistance = 1000.0f;

    if (mDistance < 0.1f)
        mDistance = 0.1f;

    if (inputEvent.mScrollY)
    {
        SetupCameraView();
    }
}

void OrbitCameraController::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void OrbitCameraController::HandleInputEvent(KeyCharEvent& inputEvent)
{
}

void OrbitCameraController::SetupCameraView()
{
    debug_assert(mSceneCamera);
    mSceneCamera->ResetOrientation();

    glm::vec3 campos;
    campos.x = glm::cos(glm::radians(mDegYaw)) * glm::sin(glm::radians(mDegPitch)) * mDistance;
    campos.y = glm::cos(glm::radians(mDegPitch)) * mDistance;
    campos.z = glm::sin(glm::radians(mDegYaw)) * glm::sin(glm::radians(mDegPitch)) * mDistance;

    campos += mFocusPoint;

    mSceneCamera->SetPosition(campos);
    mSceneCamera->FocusAt(mFocusPoint);
}