#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiView.h"
#include "GameSessionDefs.h"
#include "FrontendDefs.h"

//////////////////////////////////////////////////////////////////////////

class FrontendUi final: public UiView
{
public:
    
    //////////////////////////////////////////////////////////////////////////

    enum eMenuPage
    {
        eMenuPage_Main,
        eMenuPage_SinglePlayer,
        eMenuPage_SkirmishMaps,
        eMenuPage_MyPetDungeon,
        eMenuPage_MissionBriefing,
        eMenuPage_QuitGame,
        eMenuPage_COUNT
    };

    //////////////////////////////////////////////////////////////////////////

private:

    //////////////////////////////////////////////////////////////////////////

    class MenuPage;
    class MenuPageMain;
    class MenuPageQuitGame;
    class MenuPageSinglePlayer;
    class MenuPageSkirmishMaps;
    class MenuPageMyPetDungeon;
    class MenuPageMissionBriefing;

    //////////////////////////////////////////////////////////////////////////

public:
    FrontendUi(FrontendController& frontend);
    ~FrontendUi();

    // configuration
    void ConfigureSkirmishMaps(cxx::span<ScenarioLevelInfo> mapsList);
    void ConfigureMyPetDungeonMaps(cxx::span<ScenarioLevelInfo> mapsList);
    void ConfigureMissionBriefing(const ScenarioLevelInfo& levelInfo);

    // control
    void ShowMenuPage(eMenuPage pageId);

    // override UiView
    bool LoadContent() override;
    void Cleanup() override;
    void InputEvent(KeyInputEvent& inputEvent) override;
    void UpdateFrame(float deltaTime) override;

private:
    void RegisterPage(MenuPage* page);
    // override UiView
    void OnActivated() override;
    void OnDeactivated() override;
private:
    FrontendController& mFrontend;

    // pages
    std::unique_ptr<MenuPageMain> mPageMain;
    std::unique_ptr<MenuPageQuitGame> mPageQuit;
    std::unique_ptr<MenuPageSinglePlayer> mPageSinglePlayer;
    std::unique_ptr<MenuPageSkirmishMaps> mPageSkirmishMaps;
    std::unique_ptr<MenuPageMyPetDungeon> mPageMyPetDungeon;
    std::unique_ptr<MenuPageMissionBriefing> mPageMissionBriefing;

    MenuPage* mPages[eMenuPage_COUNT];
    MenuPage* mCurrentPage = nullptr;
};

//////////////////////////////////////////////////////////////////////////