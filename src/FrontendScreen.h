#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiView.h"
#include "GameSessionDefs.h"
#include "FrontendMenuPage.h"

//////////////////////////////////////////////////////////////////////////

class FrontendScreen: public UiView
{
public:
    FrontendScreen(FrontendController& frontend);

    // configuration
    void ConfigureSkirmishMaps(cxx::span<ScenarioLevelInfo> mapsList);
    void ConfigureMyPetDungeonMaps(cxx::span<ScenarioLevelInfo> mapsList);
    void ConfigureMissionBriefing(const ScenarioLevelInfo& levelInfo);

    // control
    void ShowMenuPage(eFrontendMenuPage pageId);

    // override UiView
    bool LoadContent() override;
    void Cleanup() override;
    void InputEvent(KeyInputEvent& inputEvent) override;
    void UpdateFrame(float deltaTime) override;

private:
    void RegisterPage(FrontendMenuPage* page);
    // override UiView
    void OnActivated() override;
    void OnDeactivated() override;
private:
    FrontendController& mFrontend;

    // pages
    FrontendMenuPage_Main mPageMain;
    FrontendMenuPage_QuitGame mPageQuit;
    FrontendMenuPage_SinglePlayer mPageSinglePlayer;
    FrontendMenuPage_SkirmishMaps mPageSkirmishMaps;
    FrontendMenuPage_MyPetDungeon mPageMyPetDungeon;
    FrontendMenuPage_MissionBriefing mPageMissionBriefing;

    FrontendMenuPage* mPages[eFrontendMenuPage_COUNT];
    FrontendMenuPage* mCurrentPage = nullptr;
};

//////////////////////////////////////////////////////////////////////////