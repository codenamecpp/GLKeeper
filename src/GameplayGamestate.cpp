#include "pch.h"
#include "GameplayGamestate.h"
#include "RenderScene.h"
#include "System.h"
#include "GameMain.h"
#include "RenderManager.h"

void GameplayGamestate::HandleGamestateEnter()
{
    gRenderScene.SetCameraControl(&mGodModeCameraControl);

    mGodModeCameraControl.SetFocusPoint(glm::vec3(0.0f));

    gGameMain.mFpsWindow.SetWindowShown(true);
}

void GameplayGamestate::HandleGamestateLeave()
{
    gRenderScene.SetCameraControl(nullptr);

    gGameMain.mFpsWindow.SetWindowShown(false);
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