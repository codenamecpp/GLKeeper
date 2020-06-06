#include "pch.h"
#include "GameplayGamestate.h"
#include "RenderScene.h"
#include "System.h"
#include "GameMain.h"
#include "RenderManager.h"
#include "GameWorld.h"

void GameplayGamestate::HandleGamestateEnter()
{
    gRenderScene.SetCameraController(&mTopDownCameraControl);

    // setup camera
    PlayerDefinition& playerDefinition = gGameWorld.mScenarioData.mPlayerDefs[ePlayerID_Keeper1];
    glm::vec3 cameraTileCoord;
    Point cameraTilePosition(playerDefinition.mStartCameraX, playerDefinition.mStartCameraY);
    GetMapBlockCenter(cameraTilePosition, cameraTileCoord);
    mTopDownCameraControl.SetFocusPoint(cameraTileCoord);

    gGameMain.mFpsWindow.SetWindowShown(true);

    gGameWorld.EnterWorld();
    mMapInteractionControl.SetFreeModeInteraction();

    mGameplayGameScreen.ShowScreen();
}

void GameplayGamestate::HandleGamestateLeave()
{
    mMapInteractionControl.ResetState();
    gGameWorld.ClearWorld();
    gRenderScene.SetCameraController(nullptr);

    gGameMain.mFpsWindow.SetWindowShown(false);

    mGameplayGameScreen.CleanupScreen();
}

void GameplayGamestate::HandleUpdateFrame()
{
    mMapInteractionControl.UpdateFrame();
}

void GameplayGamestate::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    mMapInteractionControl.ProcessInputEvent(inputEvent);
}

void GameplayGamestate::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    mMapInteractionControl.ProcessInputEvent(inputEvent);
}

void GameplayGamestate::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    mMapInteractionControl.ProcessInputEvent(inputEvent);
}

void GameplayGamestate::HandleInputEvent(KeyInputEvent& inputEvent)
{   
    if (inputEvent.HasPressed(eKeycode_F3))
    {
        gRenderManager.ReloadRenderPrograms();
        inputEvent.SetConsumed(true);
    }
    if (inputEvent.HasPressed(eKeycode_F5))
    {
        mGameplayGameScreen.ReloadScreen();
        inputEvent.SetConsumed(true);
    }

    mMapInteractionControl.ProcessInputEvent(inputEvent);
}

void GameplayGamestate::HandleInputEvent(KeyCharEvent& inputEvent)
{
    mMapInteractionControl.ProcessInputEvent(inputEvent);
}

void GameplayGamestate::HandleScreenResolutionChanged()
{
    mTopDownCameraControl.HandleScreenResolutionChanged();
}

bool GameplayGamestate::IsMapInteractionActive() const
{
    return true;
}
