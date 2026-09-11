#include "stdafx.h"
#include "FrontendUi.h"
#include "FrontendController.h"
#include "GameMain.h"
#include "Version.h"
#include "UiTextBox.h"
#include "FrontendUiControls.h"

//////////////////////////////////////////////////////////////////////////

static const std::string uiscreen_json_path = "ui/frontend_screen.json";

//////////////////////////////////////////////////////////////////////////

FrontendUi::FrontendUi(FrontendController& frontend)
    : mPages{}
    , mFrontend(frontend)
{
    mPageMain = std::make_unique<MenuPageMain>(frontend);
    mPageQuit = std::make_unique<MenuPageQuitGame>(frontend);
    mPageSinglePlayer = std::make_unique<MenuPageSinglePlayer>(frontend);
    mPageSkirmishMaps = std::make_unique<MenuPageSkirmishMaps>(frontend);
    mPageMyPetDungeon = std::make_unique<MenuPageMyPetDungeon>(frontend);
    mPageMissionBriefing = std::make_unique<MenuPageMissionBriefing>(frontend);

    RegisterPage(mPageMain.get());
    RegisterPage(mPageQuit.get());
    RegisterPage(mPageSinglePlayer.get());
    RegisterPage(mPageSkirmishMaps.get());
    RegisterPage(mPageMyPetDungeon.get());
    RegisterPage(mPageMissionBriefing.get());
}

FrontendUi::~FrontendUi()
{
    mPageMain.reset();
    mPageQuit.reset();
    mPageSinglePlayer.reset();
    mPageSkirmishMaps.reset();
    mPageMyPetDungeon.reset();
    mPageMissionBriefing.reset();
}

void FrontendUi::ShowMenuPage(eMenuPage pageId)
{
    cxx_assert(pageId < eMenuPage_COUNT);
    if (pageId < eMenuPage_COUNT)
    {
        MenuPage* nextPage = mPages[pageId];
        cxx_assert(nextPage);
        if ((nextPage == nullptr) || (nextPage == mCurrentPage))
            return;

        MenuPage* prevPage = mCurrentPage;
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

void FrontendUi::ConfigureSkirmishMaps(cxx::span<ScenarioLevelInfo> mapsList)
{
    mPageSkirmishMaps->ConfigureMaps(mapsList);
}

void FrontendUi::ConfigureMyPetDungeonMaps(cxx::span<ScenarioLevelInfo> mapsList)
{
    mPageMyPetDungeon->ConfigureLevels(mapsList);
}

void FrontendUi::ConfigureMissionBriefing(const ScenarioLevelInfo& levelInfo)
{
    mPageMissionBriefing->ConfigureBriefing(levelInfo);
}

bool FrontendUi::LoadContent()
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
            for (MenuPage* pagesRoller: mPages)
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

void FrontendUi::Cleanup()
{
    UiView::Cleanup();

    for (MenuPage* pagesRoller: mPages)
    {
        if (pagesRoller)
        {
            pagesRoller->CleanupPageContent();
        }
    }

    mCurrentPage = nullptr;
}

void FrontendUi::InputEvent(KeyInputEvent& inputEvent)
{

}

void FrontendUi::UpdateFrame(float deltaTime)
{
    UiView::UpdateFrame(deltaTime);
}

void FrontendUi::OnActivated()
{
}

void FrontendUi::OnDeactivated()
{

}

void FrontendUi::RegisterPage(MenuPage* page)
{
    cxx_assert(page);

    const eMenuPage pageId = page->GetPageId();

    cxx_assert(mPages[pageId] == nullptr);
    mPages[pageId] = page;
}
