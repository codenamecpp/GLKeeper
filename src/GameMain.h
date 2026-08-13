#pragma once

//////////////////////////////////////////////////////////////////////////

#include "ConsoleScreenView.h"
#include "LoadingScreenView.h"
#include "TitleScreenView.h"
#include "TestScreenView.h"
#include "GameSessionDefs.h"
#include "GameEvent.h"

//////////////////////////////////////////////////////////////////////////

class GameMain: private GameEventListener, private GameLoadingAware
{
public:
    // one-time initialization/deinitialization
    bool Initialize();
    void Shutdown();

    // entry point
    void Run();

    // change game state requests
    void RequestReturnToFrontend();

    // set exit request flag, execution will be interrupted soon
    void RequestQuit();

    // abnormal shutdown due to critical failure
    void Terminate();

    inline eGamestate GetCurrentGamestate() const { return mCurrentGamestate; }

    // Common processing
    void UpdateFrame();
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);

    // Show or hide developers console screen
    void OpenConsoleScreen();
    void HideConsoleScreen();

public:
    // notifications
    void ScreenResolutionChanged();

private:
    void StartCampaignScenario();
    void StartSkirmishScenario();
    void StartMPDScenario();

    bool StartScenario(const std::string& scenarioName);
    bool StartFrontend();

    void MiniUpdateFrame();

    void SetGamestate(eGamestate newGamestate);

    void UpdateLogic(float stepDeltaTime);
    void UpdatePhysics(float stepDeltaTime);

    // override GameLoadingAware
    void UpdateLoadingProgress(float progress) override;

    // override GameEventListener
    void HandleGameEvent(const GameEvent& eventData) override;

private:
    bool mQuitRequested = false;

    // gamestates
    eGamestate mCurrentGamestate = eGamestate::None;

    // screens
    ConsoleScreenView mConsoleScreen;
    LoadingScreenView mLoadingScreen;
    TitleScreenView mTitleScreen;
    TestScreenView mTestScreen;
};

//////////////////////////////////////////////////////////////////////////

extern GameMain gGame;

//////////////////////////////////////////////////////////////////////////