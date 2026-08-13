#include "stdafx.h"
#include "FrontendController.h"
#include "GameWorld.h"
#include "GameMain.h"
#include "MapUtils.h"
#include "GameEventBus.h"
#include "GameSession.h"
#include "Scene.h"

FrontendController::FrontendController()
    : mMenuScreen(*this)
{
}

void FrontendController::OnStartSinglePlayerGameSelected()
{
    gGameEventBus.Send_StartScenarioRequest("level1");
    //gGameEventBus.Send_StartScenarioRequest("Devmap");
}

void FrontendController::OnQuitGameSelected()
{
    gGameEventBus.Send_QuitGameRequest();
}

void FrontendController::OnSessionLoaded()
{
    Player& localPlayer = gGameSession.GetLocalPlayer();

    // setup camera
    mCameraController.ResetCamera();

    glm::vec3 cameraTileCoord = MapUtils::ComputeTileCenter(localPlayer.GetStartCameraTilePosition());

    cameraTileCoord[1] = 1.65f; // height
    cameraTileCoord[2] -= 0.5f;
    mCameraController.SetStartPosition(cameraTileCoord);
    mCameraController.CaptureCamera(&gScene.GetCamera());
}

void FrontendController::OnSessionStart()
{
    if (!mMenuScreen.IsActive())
    {
        mMenuScreen.Activate();
    }
}

void FrontendController::OnSessionShutdown()
{
    mCameraController.ReleaseCamera();
    if (mMenuScreen.IsActive())
    {
        mMenuScreen.Deactivate();
        mMenuScreen.Cleanup();
    }
}

void FrontendController::UpdateFrame(float deltaTime)
{
    mCameraController.UpdateFrame(deltaTime);
}

void FrontendController::UpdateLogic(float stepDeltaTime)
{

}

void FrontendController::InputEvent(MouseButtonInputEvent& inputEvent)
{
    mCameraController.InputEvent(inputEvent);
}

void FrontendController::InputEvent(KeyInputEvent& inputEvent)
{
    mCameraController.InputEvent(inputEvent);
}

void FrontendController::InputEvent(MouseMovedInputEvent& inputEvent)
{
    mCameraController.InputEvent(inputEvent);
}

void FrontendController::InputEvent(MouseScrollInputEvent& inputEvent)
{
    mCameraController.InputEvent(inputEvent);
}