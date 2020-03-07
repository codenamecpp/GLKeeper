#include "pch.h"
#include "KeeperGame.h"
#include "GameScene.h"
#include "Console.h"
#include "GenericGamestate.h"
#include "DebugConsoleWindow.h"
#include "System.h"

KeeperGame gKeeperGame;

bool KeeperGame::Initialize()
{
    if (!gGameScene.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize game scene");

        Deinit();
        return false;
    }

    return true;
}

void KeeperGame::Deinit()
{
    gGameScene.Deinit();
}

void KeeperGame::UpdateFrame()
{
    gGameScene.UpdateFrame();

    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleGamestateFrame();
    }
}

void KeeperGame::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
}

void KeeperGame::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
}

void KeeperGame::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
}

void KeeperGame::HandleInputEvent(KeyInputEvent& inputEvent)
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
}

void KeeperGame::HandleInputEvent(KeyCharEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
}