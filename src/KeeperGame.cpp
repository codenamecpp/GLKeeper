#include "pch.h"
#include "KeeperGame.h"
#include "GameScene.h"
#include "Console.h"

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
}

void KeeperGame::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void KeeperGame::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void KeeperGame::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void KeeperGame::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void KeeperGame::HandleInputEvent(KeyCharEvent& inputEvent)
{
}