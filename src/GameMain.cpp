#include "pch.h"
#include "GameMain.h"
#include "GameScene.h"
#include "Console.h"
#include "GenericGamestate.h"
#include "DebugConsoleWindow.h"
#include "System.h"

GameMain gGameMain;

bool GameMain::Initialize()
{
    if (!gGameScene.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize game scene");

        Deinit();
        return false;
    }

    return true;
}

void GameMain::Deinit()
{
    gGameScene.Deinit();
}

void GameMain::UpdateFrame()
{
    gGameScene.UpdateFrame();

    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleGamestateFrame();
    }
}

void GameMain::DebugRenderFrame(DebugRenderer& renderer)
{
    gGameScene.DebugRenderFrame(renderer);
}

void GameMain::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gGameScene.HandleInputEvent(inputEvent);
}

void GameMain::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gGameScene.HandleInputEvent(inputEvent);
}

void GameMain::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gGameScene.HandleInputEvent(inputEvent);
}

void GameMain::HandleInputEvent(KeyInputEvent& inputEvent)
{
    // show console
    if (inputEvent.HasPressed(eKeycode_TILDE))
    {
        gConsoleWindow.ToggleWindowShown();

        inputEvent.SetConsumed();
        return;
    }

    // exit game
    if (inputEvent.HasPressed(eKeycode_ESCAPE))
    {
        gSystem.QuitRequest();

        inputEvent.SetConsumed();
        return;
    }

    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gGameScene.HandleInputEvent(inputEvent);
}

void GameMain::HandleInputEvent(KeyCharEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gGameScene.HandleInputEvent(inputEvent);
}