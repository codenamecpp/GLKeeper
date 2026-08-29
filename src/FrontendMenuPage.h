#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiDefs.h"
#include "UiEvent.h"
#include "FrontendDefs.h"

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class FrontendMenuPage: public UiEventListener
{
public:
    FrontendMenuPage(FrontendController& frontend, eFrontendMenuPage pageId, std::string_view pageRootName);
    inline eFrontendMenuPage GetPageId() const { return mPageId; }
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
    eFrontendMenuPage mPageId;
    std::string mPageRootName;
    UiHierarchy* mHierarchy {};
    UiWidget* mPageRoot {};
    UiWidget* mPageConfirmButton {};
    UiWidget* mPageCancelButton {};
    UiWidget* mMainLogo {};
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class FrontendMenuPage_Main: public FrontendMenuPage
{
public:
    FrontendMenuPage_Main(FrontendController& frontend);
    // override FrontendMenuPage
    bool BindPageControls(UiHierarchy* hier) override;
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class FrontendMenuPage_QuitGame: public FrontendMenuPage
{
public:
    FrontendMenuPage_QuitGame(FrontendController& frontend);
    // override FrontendMenuPage
    void ShowPage() override;
    void PageCancelled() override;
    void PageConfirmed() override;
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class FrontendMenuPage_SinglePlayer: public FrontendMenuPage
{
public:
    FrontendMenuPage_SinglePlayer(FrontendController& frontend);
    // override FrontendMenuPage
    bool BindPageControls(UiHierarchy* hier) override;
    void ShowPage() override;
    void PageCancelled() override;
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class FrontendMenuPage_SkirmishMaps: public FrontendMenuPage
{
public:
    FrontendMenuPage_SkirmishMaps(FrontendController& frontend);
    void ConfigureMaps(cxx::span<ScenarioLevelInfo> mapsList);
    void ResetMapsScroll();
    void ResetMapsSelectedItem();
    void SetSelectedItem(int itemIndex);
    // override FrontendMenuPage
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

class FrontendMenuPage_MyPetDungeon: public FrontendMenuPage
{
public:
    FrontendMenuPage_MyPetDungeon(FrontendController& frontend);
    void ConfigureLevels(cxx::span<ScenarioLevelInfo> levelsList);
    // override FrontendMenuPage
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

class FrontendMenuPage_MissionBriefing: public FrontendMenuPage
{
public:
    FrontendMenuPage_MissionBriefing(FrontendController& frontend);
    void ConfigureBriefing(const ScenarioLevelInfo& levelInfo);
    // override FrontendMenuPage
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