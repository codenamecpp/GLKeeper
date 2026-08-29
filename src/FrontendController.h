#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameSessionController.h"
#include "FrontendCameraController.h"
#include "FrontendScreen.h"
#include "FrontendDefs.h"

//////////////////////////////////////////////////////////////////////////

// Main menu state controller
class FrontendController: public GameSessionController
{
public:
    FrontendController();

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

public:
    // frontendscreen notifications
    void OnOpenSinglePlayerMenuSelected();
    void OnMyPetDungeonMenuSelected();
    void OnMyPetDungeonMenuCancelled();
    void OnMyPetDungeonLevelSelect(const std::string& fileName);
    void OnOpenSkirmishMenuSelected();
    void OnSinglePlayerCancelled();
    void OnSkirmishMapSelectCancelled();
    void OnSkirmishMapSelectConfirmed(const std::string& fileName);
    void OnMissionBriefingCancelled(bool isMyPetDungeon);
    void OnMissionBriefingConfirmed(const std::string& fileName);
    void OnQuitGameConfirmed();
    void OnQuitGameCancelled();
    void OnQuitGameSelected();

private:
    FrontendCameraController mCameraController;
    FrontendScreen mFrontendScreen;
};

//////////////////////////////////////////////////////////////////////////