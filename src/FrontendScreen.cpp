#include "stdafx.h"
#include "FrontendScreen.h"
#include "FrontendController.h"
#include "GameMain.h"
#include "Version.h"
#include "UiTextBox.h"

//////////////////////////////////////////////////////////////////////////

static const std::string uiscreen_json_path = "ui/frontend_screen.json";

//////////////////////////////////////////////////////////////////////////

FrontendScreen::FrontendScreen(FrontendController& frontend)
    : mPages{}
    , mFrontend(frontend)
    , mPageMain(frontend)
    , mPageQuit(frontend)
    , mPageSinglePlayer(frontend)
    , mPageSkirmishMaps(frontend)
    , mPageMyPetDungeon(frontend)
    , mPageMissionBriefing(frontend)
{
    RegisterPage(&mPageMain);
    RegisterPage(&mPageQuit);
    RegisterPage(&mPageSinglePlayer);
    RegisterPage(&mPageSkirmishMaps);
    RegisterPage(&mPageMyPetDungeon);
    RegisterPage(&mPageMissionBriefing);
}

void FrontendScreen::ShowMenuPage(eFrontendMenuPage pageId)
{
    cxx_assert(pageId < eFrontendMenuPage_COUNT);
    if (pageId < eFrontendMenuPage_COUNT)
    {
        FrontendMenuPage* nextPage = mPages[pageId];
        cxx_assert(nextPage);
        if ((nextPage == nullptr) || (nextPage == mCurrentPage))
            return;

        FrontendMenuPage* prevPage = mCurrentPage;
        mCurrentPage = nullptr;
        if (prevPage)
        {
            prevPage->HidePage();
        }
        cxx_assert(mCurrentPage == nullptr);
        mCurrentPage = nextPage;
        mCurrentPage->ShowPage();
    }
}

void FrontendScreen::ConfigureSkirmishMaps(cxx::span<ScenarioLevelInfo> mapsList)
{
    mPageSkirmishMaps.ConfigureMaps(mapsList);
}

void FrontendScreen::ConfigureMyPetDungeonMaps(cxx::span<ScenarioLevelInfo> mapsList)
{
    mPageMyPetDungeon.ConfigureLevels(mapsList);
}

void FrontendScreen::ConfigureMissionBriefing(const ScenarioLevelInfo& levelInfo)
{
    mPageMissionBriefing.ConfigureBriefing(levelInfo);
}

bool FrontendScreen::LoadContent()
{
    if (!UiView::LoadContent())
    {
        if (mHierarchy.LoadFrom(uiscreen_json_path))
        {
            // bind common widgets
            if (UiWidget* uiWidget = mHierarchy.FindWidgetWithName("version_string"))
            {
                UiTextBox* textBox = (UiTextBox*) uiWidget;

                // set version string
                std::wstring versionNumber {GAME_VERSION_STRING, GAME_VERSION_STRING + sizeof(GAME_VERSION_STRING)};
                versionNumber.insert(versionNumber.begin(), L'V');
                textBox->SetText(versionNumber);
            }

            // pages
            for (FrontendMenuPage* pagesRoller: mPages)
            {
                bool isSuccess = false;
                if (pagesRoller)
                {
                    isSuccess = pagesRoller->BindPageControls(&mHierarchy);
                }
                cxx_assert(isSuccess);
            }
        }
        else
        {
            cxx_assert(false);
        }
    }
    return IsHierarchyLoaded();
}

void FrontendScreen::Cleanup()
{
    UiView::Cleanup();

    for (FrontendMenuPage* pagesRoller: mPages)
    {
        if (pagesRoller)
        {
            pagesRoller->CleanupPageContent();
        }
    }

    mCurrentPage = nullptr;
}

void FrontendScreen::InputEvent(KeyInputEvent& inputEvent)
{

}

void FrontendScreen::UpdateFrame(float deltaTime)
{
    UiView::UpdateFrame(deltaTime);
}

void FrontendScreen::OnActivated()
{
}

void FrontendScreen::OnDeactivated()
{

}

void FrontendScreen::RegisterPage(FrontendMenuPage* page)
{
    cxx_assert(page);

    const eFrontendMenuPage pageId = page->GetPageId();

    cxx_assert(mPages[pageId] == nullptr);
    mPages[pageId] = page;
}
