#include "pch.h"
#include "GameMain.h"
#include "RenderScene.h"
#include "Console.h"
#include "GenericGamestate.h"
#include "ToolsUIConsoleWindow.h"
#include "System.h"
#include "GameWorld.h"
#include "ToolsUIManager.h"

GameMain gGameMain;

bool GameMain::Initialize()
{
    if (!gRenderScene.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize game scene");

        Deinit();
        return false;
    }

    gToolsUIManager.AttachWindow(&mFpsWindow);
    mFpsWindow.SetWindowShown(false);

    if (!gGameWorld.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize game world");

        Deinit();
        return false;
    }

    // set initial gamestate
    //SwitchToGameState(&mMeshViewGamestate);
    if (!gSystem.mStartupParams.mStartupMapName.empty())
    {
        if (gGameWorld.LoadScenario(gSystem.mStartupParams.mStartupMapName))
        {
            SwitchToGameState(&mGameplayGamestate);
        }
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Info, "Startup map is not specified");
    }

    //SwitchToGameState(&mGuiTestGamestate);

    return true;
}

void GameMain::Deinit()
{
    gGameWorld.Deinit();

    SwitchToGameState(nullptr);

    gToolsUIManager.DetachWindow(&mFpsWindow);
    gRenderScene.Deinit();
}

void GameMain::UpdateFrame()
{
    gRenderScene.UpdateFrame();

    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleUpdateFrame();
    }

    gGameWorld.UpdateFrame();
}

void GameMain::DebugRenderFrame(DebugRenderer& renderer)
{
    gRenderScene.DebugRenderFrame(renderer);
}

void GameMain::ProcessInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gRenderScene.ProcessInputEvent(inputEvent);
}

void GameMain::ProcessInputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gRenderScene.ProcessInputEvent(inputEvent);
}

void GameMain::ProcessInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gRenderScene.ProcessInputEvent(inputEvent);
}

void GameMain::ProcessInputEvent(KeyInputEvent& inputEvent)
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
    gRenderScene.ProcessInputEvent(inputEvent);
}

void GameMain::ProcessInputEvent(KeyCharEvent& inputEvent)
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleInputEvent(inputEvent);
    }
    gRenderScene.ProcessInputEvent(inputEvent);
}

bool GameMain::IsMeshViewGamestate() const
{
    return mCurrentGamestate == &mMeshViewGamestate;
}

bool GameMain::IsGameplayGamestate() const
{
    return mCurrentGamestate == &mGameplayGamestate;
}

bool GameMain::IsGuiTestGamestate() const
{
    return mCurrentGamestate == &mGuiTestGamestate;
}

void GameMain::HandleScreenResolutionChanged()
{
    if (mCurrentGamestate)
    {
        mCurrentGamestate->HandleScreenResolutionChanged();
    }
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
