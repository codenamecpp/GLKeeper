#include "stdafx.h"
#include "Camera.h"
#include "FrontendCameraController.h"
#include "GameWorld.h"
#include "CameraEffects.h"
#include "FrontendController.h"

//////////////////////////////////////////////////////////////////////////

#define FRONT_END_CAMERA_NEAR 0.1f
#define FRONT_END_CAMERA_FAR 10.0f
#define FRONT_END_CAMERA_FOVY 65.0f

//////////////////////////////////////////////////////////////////////////

FrontendCameraController::FrontendCameraController(FrontendController& froented)
    : mFrontend(froented)
    , mStartPosition()
{
    InitTransitionsMatrix();
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

    gCameraEffects.FinishTransition();

    mCurrentMode = eWorkMode_Default;
    mCurrentLocation = eLocation_Entrance;
}

void FrontendCameraController::UpdateFrame(float deltaTime)
{
    // check transition completed
    if (mCurrentMode == eWorkMode_InTransition)
    {
        if (!gCameraEffects.InTransition())
        {
            mCurrentMode = eWorkMode_Default;
            mFrontend.OnCameraTransitionCompleted();
        }
    }
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
    if (mCamera == nullptr) 
        return;

    Camera::ProjectionParams cameraParams (FRONT_END_CAMERA_NEAR, FRONT_END_CAMERA_FAR, FRONT_END_CAMERA_FOVY);
    mCamera->SetupProjection(cameraParams);
    mCamera->ResetOrientation();
    mCamera->SetPosition(mStartPosition);

    gCameraEffects.StartTransition(CameraPathId_Frontend_Intro, mStartPosition);
    StopCamera();

    mCurrentLocation = eLocation_Entrance;
}

void FrontendCameraController::StopCamera()
{
    gCameraEffects.FinishTransition();

    mCurrentMode = eWorkMode_Default;
}

void FrontendCameraController::SetStartPosition(const glm::vec3& position)
{
    mStartPosition = position;
}

void FrontendCameraController::StartTransitionToLocation(eLocation newLocation)
{
    cxx_assert(newLocation < eLocation_COUNT);
    CameraPathId pathId = mTransitionsMatrix[mCurrentLocation][newLocation];
    gCameraEffects.StartTransition(pathId, mStartPosition);
    mCurrentMode = eWorkMode_InTransition;
    mCurrentLocation = newLocation;
}

void FrontendCameraController::InitTransitionsMatrix()
{
    auto setDefaultDestination = [this](eLocation dstLocation, CameraPathId pathId)
        {
            for (auto& roller: this->mTransitionsMatrix)
            {
                roller[dstLocation] = pathId;
            }
        };
    // defaults
    setDefaultDestination(eLocation_Entrance    , CameraPathId_Frontend_StaticEntry);
    setDefaultDestination(eLocation_Table       , CameraPathId_Frontend_EntryToTable);
    setDefaultDestination(eLocation_1stRight    , CameraPathId_Frontend_Static1stRight);
    setDefaultDestination(eLocation_1stLeft     , CameraPathId_Frontend_Static1stLeft);
    setDefaultDestination(eLocation_2ndRight    , CameraPathId_Frontend_Static2ndRight);
    setDefaultDestination(eLocation_2ndLeft     , CameraPathId_Frontend_Static2ndLeft);
    setDefaultDestination(eLocation_CreditView  , CameraPathId_Frontend_1stLeftToCreditView);

    // exact
    mTransitionsMatrix[eLocation_Entrance][eLocation_Table]     = CameraPathId_Frontend_EntryToTable;
    mTransitionsMatrix[eLocation_Table][eLocation_Entrance]     = CameraPathId_Frontend_TableToEntry;
    mTransitionsMatrix[eLocation_Table][eLocation_1stRight]     = CameraPathId_Frontend_TableTo1stRight;
    mTransitionsMatrix[eLocation_1stRight][eLocation_Table]     = CameraPathId_Frontend_1stRightToTable;
    mTransitionsMatrix[eLocation_Entrance][eLocation_2ndRight]  = CameraPathId_Frontend_EntryTo2ndRight;
    mTransitionsMatrix[eLocation_2ndRight][eLocation_Entrance]  = CameraPathId_Frontend_2ndRightToEntry;
    mTransitionsMatrix[eLocation_Entrance][eLocation_1stLeft]   = CameraPathId_Frontend_EntryTo1stLeft;
    mTransitionsMatrix[eLocation_1stLeft][eLocation_Entrance]   = CameraPathId_Frontend_1stLeftToEntry;
    mTransitionsMatrix[eLocation_Entrance][eLocation_2ndLeft]   = CameraPathId_Frontend_EntryTo2ndLeft;
    mTransitionsMatrix[eLocation_2ndLeft][eLocation_Entrance]   = CameraPathId_Frontend_2ndLeftToEntry;
    mTransitionsMatrix[eLocation_1stLeft][eLocation_CreditView] = CameraPathId_Frontend_1stLeftToCreditView;
    mTransitionsMatrix[eLocation_CreditView][eLocation_1stLeft] = CameraPathId_Frontend_CreditViewTo1stLeft;
    mTransitionsMatrix[eLocation_Entrance][eLocation_1stRight]  = CameraPathId_Frontend_EntryTo1stRight;
    mTransitionsMatrix[eLocation_1stRight][eLocation_Entrance]  = CameraPathId_Frontend_1stRightToEntry;
}

