#include "pch.h"
#include "GameMain.h"
#include "RenderScene.h"
#include "Console.h"
#include "GenericGamestate.h"
#include "ConsoleWindow.h"
#include "System.h"
#include "GameWorld.h"

GameMain gGameMain;

bool GameMain::Initialize()
{
    if (!gRenderScene.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize game scene");

        Deinit();
        return false;
    }

    // set initial gamestate
    //SwitchToGameState(&mMeshViewGamestate);

    gGameWorld.InitializeWorld("temp");

    SwitchToGameState(&mGameplayGamestate);

    return true;
}

void GameMain::Deinit()
{
    gGameWorld.Deinit();

    SwitchToGameState(nullptr);

    gRenderScene.Deinit();
}

void GameMain::UpdateFrame()
{
    gRenderScene.UpdateFrame();

    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleUpdateFrame();
    }
}

void GameMain::DebugRenderFrame(DebugRenderer& renderer)
{
    gRenderScene.DebugRenderFrame(renderer);
}

void GameMain::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gRenderScene.HandleInputEvent(inputEvent);
}

void GameMain::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gRenderScene.HandleInputEvent(inputEvent);
}

void GameMain::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gRenderScene.HandleInputEvent(inputEvent);
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
    gRenderScene.HandleInputEvent(inputEvent);
}

void GameMain::HandleInputEvent(KeyCharEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gRenderScene.HandleInputEvent(inputEvent);
}

bool GameMain::IsMeshViewGamestate() const
{
    return mCurrentGamestate == &mMeshViewGamestate;
}

bool GameMain::IsGameplayGamestate() const
{
    return mCurrentGamestate == &mGameplayGamestate;
}

void GameMain::SwitchToGameState(GenericGamestate* gamestate)
{
    if (mCurrentGamestate == gamestate)
        return;

    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleGamestateLeave();
    }
    mCurrentGamestate = gamestate;
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleGamestateEnter();
    }
}
