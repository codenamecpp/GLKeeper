#include "stdafx.h"
#include "Camera.h"
#include "FrontendCameraController.h"
#include "GameWorld.h"

//////////////////////////////////////////////////////////////////////////

#define FRONT_END_CAMERA_NEAR 0.1f
#define FRONT_END_CAMERA_FAR 10.0f
#define FRONT_END_CAMERA_FOVY 65.0f

//////////////////////////////////////////////////////////////////////////

FrontendCameraController::FrontendCameraController()
    : mStartPosition()
{
}

void FrontendCameraController::CaptureCamera(Camera* camera)
{
    mCamera = camera;
    cxx_assert(mCamera);
    ResetCamera();
}

void FrontendCameraController::ReleaseCamera()
{
    mCamera = nullptr;
}

void FrontendCameraController::UpdateFrame(float deltaTime)
{
}

void FrontendCameraController::InputEvent(MouseButtonInputEvent& inputEvent)
{
}

void FrontendCameraController::InputEvent(KeyInputEvent& inputEvent)
{
}

void FrontendCameraController::InputEvent(MouseMovedInputEvent& inputEvent)
{
}

void FrontendCameraController::InputEvent(MouseScrollInputEvent& inputEvent)
{
}

void FrontendCameraController::ResetCamera()
{
    if (mCamera == nullptr) return;

    Camera::ProjectionParams cameraParams (FRONT_END_CAMERA_NEAR, FRONT_END_CAMERA_FAR, FRONT_END_CAMERA_FOVY);
    mCamera->SetupProjection(cameraParams);
    mCamera->ResetOrientation();
    mCamera->SetPosition(mStartPosition);

    StopCamera();
}

void FrontendCameraController::StopCamera()
{
}

void FrontendCameraController::SetStartPosition(const glm::vec3& position)
{
    mStartPosition = position;
}
