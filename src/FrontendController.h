#pragma once

//////////////////////////////////////////////////////////////////////////

#include "FrontendCameraController.h"
#include "GameSessionController.h"
#include "FrontendScreen.h"

//////////////////////////////////////////////////////////////////////////

// Main menu state controller
class FrontendController: public GameSessionController
{
public:
    FrontendController();

    // notifications
    void OnStartSinglePlayerGameSelected();
    void OnQuitGameSelected();

    // override GameSessionController
    void OnSessionLoaded() override;
    void OnSessionStart() override;
    void OnSessionShutdown() override;
    void UpdateFrame(float deltaTime) override;
    void UpdateLogic(float stepDeltaTime) override;
    void InputEvent(MouseButtonInputEvent& inputEvent) override;
    void InputEvent(KeyInputEvent& inputEvent) override;
    void InputEvent(MouseMovedInputEvent& inputEvent) override;
    void InputEvent(MouseScrollInputEvent& inputEvent) override;

private:
    FrontendCameraController mCameraController;
    FrontendScreen mMenuScreen;
};
