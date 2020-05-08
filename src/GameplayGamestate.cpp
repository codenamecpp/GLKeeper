#include "pch.h"
#include "GameplayGamestate.h"
#include "RenderScene.h"
#include "System.h"
#include "GameMain.h"
#include "RenderManager.h"
#include "GameWorld.h"

void GameplayGamestate::HandleGamestateEnter()
{
    gRenderScene.SetCameraController(&mGodmodeCameraControl);

    mGodmodeCameraControl.SetFocusPoint(glm::vec3(0.0f));

    gGameMain.mFpsWindow.SetWindowShown(true);

    gGameWorld.EnterWorld();

    mGameplayGameScreen.ShowScreen();
}

void GameplayGamestate::HandleGamestateLeave()
{
    gGameWorld.ClearWorld();
    gRenderScene.SetCameraController(nullptr);

    gGameMain.mFpsWindow.SetWindowShown(false);

    mGameplayGameScreen.CleanupScreen();
}

void GameplayGamestate::HandleUpdateFrame()
{
}

void GameplayGamestate::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void GameplayGamestate::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void GameplayGamestate::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void GameplayGamestate::HandleInputEvent(KeyInputEvent& inputEvent)
{   
    if (inputEvent.HasPressed(eKeycode_F3))
    {
        gRenderManager.ReloadRenderPrograms();
        inputEvent.SetConsumed(true);
    }
}

void GameplayGamestate::HandleInputEvent(KeyCharEvent& inputEvent)
{
}

void GameplayGamestate::HandleScreenResolutionChanged()
{
    mGodmodeCameraControl.HandleScreenResolutionChanged();
}
