#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiDefs.h"
#include "UiEvent.h"
#include "FrontendDefs.h"
#include "FrontendUi.h"

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class FrontendUi::MenuPage: public UiEventListener
{
public:
    MenuPage(FrontendController& frontend, eMenuPage pageId, std::string_view pageRootName);
    inline eMenuPage GetPageId() const { return mPageId; }
    // overridables
    virtual bool BindPageControls(UiHierarchy* hier);
    virtual void CleanupPageContent();
    virtual void ShowPage();
    virtual void HidePage();
    virtual void PageCancelled();
    virtual void PageConfirmed();
protected:
    enum ePageButtons
    {
        ePageButtons_None,
        ePageButtons_Cancel,
        ePageButtons_Confirm,
        ePageButtons_Both,
    };
    void SetPageButtons(ePageButtons pageButtons);
    void ShowMainLogo(bool showLogo);
    // override UiEventListener
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;
private:
    void SubscribePageButtons(bool isSubscribe);
protected:
    FrontendController& mFrontend;
    eMenuPage mPageId;
    std::string mPageRootName;
    UiHierarchy* mHierarchy {};
    UiWidget* mPageRoot {};
    UiWidget* mPageConfirmButton {};
    UiWidget* mPageCancelButton {};
    UiWidget* mMainLogo {};
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class FrontendUi::MenuPageMain: public FrontendUi::MenuPage
{
public:
    MenuPageMain(FrontendController& frontend);
    // override MenuPage
    bool BindPageControls(UiHierarchy* hier) override;
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class FrontendUi::MenuPageQuitGame: public FrontendUi::MenuPage
{
public:
    MenuPageQuitGame(FrontendController& frontend);
    // override MenuPage
    void ShowPage() override;
    void PageCancelled() override;
    void PageConfirmed() override;
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class FrontendUi::MenuPageSinglePlayer: public FrontendUi::MenuPage
{
public:
    MenuPageSinglePlayer(FrontendController& frontend);
    // override MenuPage
    bool BindPageControls(UiHierarchy* hier) override;
    void ShowPage() override;
    void PageCancelled() override;
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class FrontendUi::MenuPageSkirmishMaps: public FrontendUi::MenuPage
{
public:
    MenuPageSkirmishMaps(FrontendController& frontend);
    void ConfigureMaps(cxx::span<ScenarioLevelInfo> mapsList);
    void ResetMapsScroll();
    void ResetMapsSelectedItem();
    void SetSelectedItem(int itemIndex);
    // override MenuPage
    bool BindPageControls(UiHierarchy* hier) override;
    void CleanupPageContent() override;
    void ShowPage() override;
    void PageCancelled() override;
    void PageConfirmed() override;
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;
private:
    void SetMapsScrollPosition(int position, bool fromScrollBar);
    void RefreshTable();
    void RefreshTableRow(int rowIndex, bool isSelected, const ScenarioLevelInfo& levelInfo);
    void RefreshSelectionInfo();
    void RefreshMapsScrollBar();
    void ShowTableRow(int rowIndex, bool isShown);
    void NewTableRow(UiWidget* table, UiWidget* tableRowTemplate);
private:
    std::vector<ScenarioLevelInfo> mMapsList;
    struct TableRow
    {
        UiWidget* mWidget {};
        UiWidget* mSelectionWidget {};
        UiTextBox* mMapNameTextBox {};
        UiTextBox* mMaxPlayersTextBox {};
        UiTextBox* mMapSizeTextBox {};
    };
    UiTextBox* mMapNameTextBox {};
    UiPicture* mMapPreviewPicture {};
    UiScrollBar* mMapsListScrollBar {};
    std::vector<TableRow> mTableRows;
    int mTableFirstItem = 0;
    int mTableSelectedItem = 0;
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class FrontendUi::MenuPageMyPetDungeon: public FrontendUi::MenuPage
{
public:
    MenuPageMyPetDungeon(FrontendController& frontend);
    void ConfigureLevels(cxx::span<ScenarioLevelInfo> levelsList);
    // override MenuPage
    bool BindPageControls(UiHierarchy* hier) override;
    void CleanupPageContent() override;
    void ShowPage() override;
    void PageCancelled() override;
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;
private:
    void RefreshLevelsList();
private:
    std::vector<ScenarioLevelInfo> mLevelsList;
    std::vector<UiTextBox*> mNamesList;
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class FrontendUi::MenuPageMissionBriefing: public FrontendUi::MenuPage
{
public:
    MenuPageMissionBriefing(FrontendController& frontend);
    void ConfigureBriefing(const ScenarioLevelInfo& levelInfo);
    // override MenuPage
    bool BindPageControls(UiHierarchy* hier) override;
    void CleanupPageContent() override;
    void ShowPage() override;
    void PageCancelled() override;
    void PageConfirmed() override;
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;
private:
    void RefreshBriefing();
private:
    ScenarioLevelInfo mLevelInfo;
    UiTextBox* mSubtitleTextBox {};
    UiTextBox* mMainObjectiveTextBox {};
    UiTextBox* mSubObjectiveTextBox {};
    std::vector<UiPicture*> mObjectivePics;
};

//////////////////////////////////////////////////////////////////////////