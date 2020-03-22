#include "pch.h"
#include "GodModeCameraControl.h"
#include "System.h"
#include "RenderScene.h"

#define CAM_DEFAULT_PITCH_ANGLE    -53.0f
#define CAM_DEFAULT_MOTION_SPEED    10.0f
#define CAM_DEFAULT_ROTATION_SPEED  90.0f
#define CAM_DEFAULT_DISTANCE        8.0f
#define CAM_DEFAULT_NEAR            0.01f
#define CAM_DEFAULT_FAR             200.0f
#define CAM_DEFAULT_FOVY            60.0f

GodModeCameraControl::GodModeCameraControl()
    : mFocusPoint()
    , mDegYaw()
    , mDegPitch(CAM_DEFAULT_PITCH_ANGLE)
    , mDistance(CAM_DEFAULT_DISTANCE)
    , mIncreasingFov()
    , mDecreasingFov()
    , mZoomingIn()
    , mZoomingOut()
    , mMovingN()
    , mMovingE()
    , mMovingS()
    , mMovingW()
    , mMovingAltMode()
    , mFOVy(CAM_DEFAULT_FOVY)
    , mIncreasingPitch()
    , mDecreasingPitch()
{
}

void GodModeCameraControl::Set3rdPersonParams(float yawDegrees, float pitchDegrees, float distance)
{
    mDegYaw = yawDegrees;
    mDegPitch = pitchDegrees;
    mDistance = distance;

    SetupCameraView();
}

void GodModeCameraControl::SetFocusPoint(const glm::vec3& position)
{
    mFocusPoint = position;

    SetupCameraView();
}

void GodModeCameraControl::StopCameraActivity()
{
    mIncreasingFov = false;
    mDecreasingFov = false;

    mZoomingIn = false;
    mZoomingOut = false;

    mIncreasingPitch = false;
    mDecreasingPitch = false;

    mMovingN = false;
    mMovingE = false;
    mMovingS = false;
    mMovingW = false;
    mMovingAltMode = false;
}

void GodModeCameraControl::HandleUpdateFrame(float dtseconds)
{
    glm::vec3 moveDirection {};
    float elevation = 0.0f;

    if (mIncreasingFov || mDecreasingFov)
    {
        if (mIncreasingFov)
        {
            mFOVy += 5.0f * dtseconds;
        }
        else
        {
            mFOVy -= 5.0f * dtseconds;
        }

        SetupCameraProjection();
    }
    
    if (mIncreasingPitch || mDecreasingPitch)
    {
        mDegPitch = cxx::normalize_angle_180(mDegPitch + (CAM_DEFAULT_ROTATION_SPEED * dtseconds) * (mDecreasingPitch ? -1.0f : 1.0f));
        mDegPitch = glm::clamp(mDegPitch, -80.0f, -1.0f);
    }

    bool rotatingCW = mMovingE && mMovingAltMode;
    bool rotatingCCW = mMovingW && mMovingAltMode;
    bool zoomingIn = (mMovingN && mMovingAltMode) || mZoomingIn;
    bool zoomingOut = (mMovingS && mMovingAltMode) || mZoomingOut;

    if (zoomingIn)
    {
        elevation = -CAM_DEFAULT_MOTION_SPEED * dtseconds;
    }

    if (zoomingOut)
    {
        elevation = CAM_DEFAULT_MOTION_SPEED * dtseconds;
    }

    if ((mMovingE || mMovingW) && !mMovingAltMode)
    {
        if (mMovingE)
        {
            moveDirection += (mSceneCamera->mRightVector * CAM_DEFAULT_MOTION_SPEED * dtseconds);
        }
        if (mMovingW)
        {
            moveDirection += (-mSceneCamera->mRightVector * CAM_DEFAULT_MOTION_SPEED * dtseconds);
        }
    }

    if ((mMovingN || mMovingS) && !mMovingAltMode)
    {
        const glm::vec3 vmove = glm::normalize(glm::cross(mSceneCamera->mRightVector, SceneAxis_Y()));
        if (mMovingN)
        {
            moveDirection += (-vmove * CAM_DEFAULT_MOTION_SPEED * dtseconds);
        }
        if (mMovingS)
        {
            moveDirection += (vmove * CAM_DEFAULT_MOTION_SPEED * dtseconds);
        }
    }

    if (rotatingCCW || rotatingCW)
    {
        mDegYaw = cxx::normalize_angle_180(mDegYaw + (CAM_DEFAULT_ROTATION_SPEED * dtseconds) * (rotatingCCW ? -1.0f : 1.0f));
    }

    mDistance += elevation;
    mFocusPoint += moveDirection;

    SetupCameraView();
}

void GodModeCameraControl::HandleSceneAttach()
{
    SetupCameraProjection();
    SetupCameraView();
    StopCameraActivity();
}

void GodModeCameraControl::HandleSceneDetach()
{
}

void GodModeCameraControl::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void GodModeCameraControl::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void GodModeCameraControl::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void GodModeCameraControl::HandleInputEvent(KeyInputEvent& inputEvent)
{
    if (inputEvent.mKeycode == eKeycode_0)
    {
        mIncreasingFov = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == eKeycode_9)
    {
        mDecreasingFov = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == eKeycode_PAGE_UP)
    {
        mZoomingIn = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == eKeycode_PAGE_DOWN)
    {
        mZoomingOut = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == eKeycode_HOME)
    {
        mIncreasingPitch = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == eKeycode_END)
    {
        mDecreasingPitch = inputEvent.mPressed;
    }

    if (inputEvent.mKeycode == eKeycode_W || inputEvent.mKeycode == eKeycode_A || 
        inputEvent.mKeycode == eKeycode_S || inputEvent.mKeycode == eKeycode_D || inputEvent.mKeycode == eKeycode_LEFT_CTRL ||
        inputEvent.mKeycode == eKeycode_RIGHT_CTRL)
    {
        mMovingAltMode = inputEvent.HasModifiers(KeyModifier_Ctrl) || 
            ((inputEvent.mKeycode == eKeycode_LEFT_CTRL || inputEvent.mKeycode == eKeycode_RIGHT_CTRL) && inputEvent.mPressed);
        mMovingN = (inputEvent.mKeycode == eKeycode_W) ? (inputEvent.mPressed) : mMovingN;
        mMovingE = (inputEvent.mKeycode == eKeycode_D) ? (inputEvent.mPressed) : mMovingE;
        mMovingS = (inputEvent.mKeycode == eKeycode_S) ? (inputEvent.mPressed) : mMovingS;
        mMovingW = (inputEvent.mKeycode == eKeycode_A) ? (inputEvent.mPressed) : mMovingW;
    }
}

void GodModeCameraControl::HandleInputEvent(KeyCharEvent& inputEvent)
{
}

void GodModeCameraControl::SetupCameraView()
{
    if (mSceneCamera == nullptr)
        return;

    mSceneCamera->ResetOrientation();

    glm::vec3 campos;
    campos.x = glm::cos(glm::radians(mDegYaw)) * glm::sin(glm::radians(mDegPitch)) * mDistance;
    campos.y = glm::cos(glm::radians(mDegPitch)) * mDistance;
    campos.z = glm::sin(glm::radians(mDegYaw)) * glm::sin(glm::radians(mDegPitch)) * mDistance;

    campos += mFocusPoint;

    mSceneCamera->SetPosition(campos);
    mSceneCamera->FocusAt(mFocusPoint);
}

void GodModeCameraControl::SetupCameraProjection()
{
    if (mSceneCamera == nullptr)
        return;

    float screenAspectRatio = gSystem.mSettings.mScreenAspectRatio;
    mSceneCamera->SetPerspective(screenAspectRatio, mFOVy, CAM_DEFAULT_NEAR, CAM_DEFAULT_FAR);
}
