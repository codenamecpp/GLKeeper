#include "stdafx.h"
#include "FrontendMenuPage.h"
#include "UiWidget.h"
#include "FrontendController.h"
#include "UiTextBox.h"
#include "UiPicture.h"
#include "UiScrollBar.h"
#include "TextManager.h"
#include "DK2AssetLoader.h"

//////////////////////////////////////////////////////////////////////////

enum FrontendActionTypeId : unsigned int
{
    FrontendActionTypeId_None = 0,
    FrontendActionTypeId_MyPetDungeonLevelSelect,
};

//////////////////////////////////////////////////////////////////////////

static const char* FrontendPagesCancel = "cancel";
static const char* FrontendPagesConfirm = "confirm";

//////////////////////////////////////////////////////////////////////////

FrontendMenuPage::FrontendMenuPage(FrontendController& frontend, eFrontendMenuPage pageId, std::string_view pageRootName)
    : mPageId(pageId)
    , mFrontend(frontend)
    , mPageRootName(pageRootName)
{
}

bool FrontendMenuPage::BindPageControls(UiHierarchy* hier)
{
    mHierarchy = hier;
    cxx_assert(mHierarchy);

    mPageRoot = mHierarchy->FindWidgetWithName(mPageRootName);
    cxx_assert(mPageRoot);

    if (mPageRoot)
    {
        mPageRoot->SetVisible(false);
    }

    if (mHierarchy)
    {
        // common buttons
        mPageCancelButton = mHierarchy->FindWidgetWithName(FrontendPagesCancel);
        if (mPageCancelButton)
        {
            mPageCancelButton->UserData().SetValue(FrontendPagesCancel);
        }
        cxx_assert(mPageCancelButton);

        mPageConfirmButton = mHierarchy->FindWidgetWithName(FrontendPagesConfirm);
        if (mPageConfirmButton)
        {
            mPageConfirmButton->UserData().SetValue(FrontendPagesConfirm);
        }
        cxx_assert(mPageConfirmButton);

        mMainLogo = mHierarchy->FindWidgetWithName("logo");
        cxx_assert(mMainLogo);
    }
    return true;
}

void FrontendMenuPage::CleanupPageContent()
{
    SubscribePageButtons(false);
    mHierarchy = nullptr;
    mPageRoot = nullptr;
    mPageConfirmButton = nullptr;
    mPageCancelButton = nullptr;
    mMainLogo = nullptr;
}

void FrontendMenuPage::ShowPage()
{
    if (mPageRoot && !mPageRoot->IsVisibleSelf())
    {
        mPageRoot->SetVisible(true);
        SetPageButtons(ePageButtons_None);
        ShowMainLogo(true);
        SubscribePageButtons(true);
    }
}

void FrontendMenuPage::HidePage()
{
    if (mPageRoot && mPageRoot->IsVisibleSelf())
    {
        mPageRoot->SetVisible(false);
        SetPageButtons(ePageButtons_None);
        SubscribePageButtons(false);
    }
}

void FrontendMenuPage::PageCancelled()
{
    cxx_assert(false);
}

void FrontendMenuPage::PageConfirmed()
{
    cxx_assert(false);
}

void FrontendMenuPage::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    const char* menuItemId = sender->UserData().GetValue<const char*>();
    if (eventDesc.IsEvent(UiEventId_OnPress))
    {
        if (menuItemId == FrontendPagesCancel)
        {
            PageCancelled();
            return;
        }

        if (menuItemId == FrontendPagesConfirm)
        {
            PageConfirmed();
            return;
        }
    }
}

void FrontendMenuPage::SubscribePageButtons(bool isSubscribe)
{
    if (UiWidget* button = mPageCancelButton)
    {
        if (isSubscribe)
        {
            button->Subscribe(this);
        }
        else
        {
            button->Unsubscribe(this);
        }
    }

    if (UiWidget* button = mPageConfirmButton)
    {
        if (isSubscribe)
        {
            button->Subscribe(this);
        }
        else
        {
            button->Unsubscribe(this);
        }
    }
}

void FrontendMenuPage::SetPageButtons(ePageButtons pageButtons)
{
    if (mPageCancelButton)
    {
        mPageCancelButton->SetVisible((pageButtons == ePageButtons_Cancel) || (pageButtons == ePageButtons_Both));
    }

    if (mPageConfirmButton)
    {
        mPageConfirmButton->SetVisible((pageButtons == ePageButtons_Confirm) || (pageButtons == ePageButtons_Both));
    }
}

void FrontendMenuPage::ShowMainLogo(bool showLogo)
{
    if (mMainLogo)
    {
        mMainLogo->SetVisible(showLogo);
    }
}

//////////////////////////////////////////////////////////////////////////

static const char* FrontendMainPageSinglePlayer = "single_player";
static const char* FrontendMainPageQuit = "quit";
static const char* FrontendMainPageMpd = "mpd";

//////////////////////////////////////////////////////////////////////////

FrontendMenuPage_Main::FrontendMenuPage_Main(FrontendController& frontend)
    : FrontendMenuPage(frontend, eFrontendMenuPage_Main, "menu_page_1")
{
}

bool FrontendMenuPage_Main::BindPageControls(UiHierarchy* hier)
{
    bool isSuccess = FrontendMenuPage::BindPageControls(hier);
    if (isSuccess)
    {
        if (UiWidget* uiWidget = mPageRoot->FindChildWithName(FrontendMainPageSinglePlayer))
        {
            uiWidget->Subscribe(this);
            uiWidget->UserData().SetValue(FrontendMainPageSinglePlayer);
        }

        if (UiWidget* uiWidget = mPageRoot->FindChildWithName(FrontendMainPageQuit))
        {
            uiWidget->Subscribe(this);
            uiWidget->UserData().SetValue(FrontendMainPageQuit);
        }

        if (UiWidget* uiWidget = mPageRoot->FindChildWithName(FrontendMainPageMpd))
        {
            uiWidget->Subscribe(this);
            uiWidget->UserData().SetValue(FrontendMainPageMpd);
        }
    }
    return isSuccess;
}

void FrontendMenuPage_Main::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    const char* menuItemId = sender->UserData().GetValue<const char*>();
    if (eventDesc.IsEvent(UiEventId_OnPress))
    {
        if (menuItemId == FrontendMainPageSinglePlayer)
        {
            mFrontend.OnOpenSinglePlayerMenuSelected();
            return;
        }

        if (menuItemId == FrontendMainPageQuit)
        {
            mFrontend.OnQuitGameSelected();
            return;
        }

        if (menuItemId == FrontendMainPageMpd)
        {
            mFrontend.OnMyPetDungeonMenuSelected();
            return;
        }
    }
    FrontendMenuPage::HandleUiEvent(sender, eventDesc);
}

//////////////////////////////////////////////////////////////////////////

FrontendMenuPage_QuitGame::FrontendMenuPage_QuitGame(FrontendController& frontend)
    : FrontendMenuPage(frontend, eFrontendMenuPage_QuitGame, "menu_page_3")
{
}

void FrontendMenuPage_QuitGame::ShowPage()
{
    FrontendMenuPage::ShowPage();
    SetPageButtons(ePageButtons_Both);
}

void FrontendMenuPage_QuitGame::PageCancelled()
{
    mFrontend.OnQuitGameCancelled();
}

void FrontendMenuPage_QuitGame::PageConfirmed()
{
    mFrontend.OnQuitGameConfirmed();
}

//////////////////////////////////////////////////////////////////////////

static const char* FrontendSinglePlayerPageSkirmish = "skirmish";

//////////////////////////////////////////////////////////////////////////

FrontendMenuPage_SinglePlayer::FrontendMenuPage_SinglePlayer(FrontendController& frontend)
    : FrontendMenuPage(frontend, eFrontendMenuPage_SinglePlayer, "menu_page_2")
{
}

bool FrontendMenuPage_SinglePlayer::BindPageControls(UiHierarchy* hier)
{
    bool isSuccess = FrontendMenuPage::BindPageControls(hier);
    if (isSuccess)
    {
        if (UiWidget* uiWidget = mPageRoot->FindChildWithName(FrontendSinglePlayerPageSkirmish))
        {
            uiWidget->Subscribe(this);
            uiWidget->UserData().SetValue(FrontendSinglePlayerPageSkirmish);
        }
    }
    return isSuccess;
}

void FrontendMenuPage_SinglePlayer::ShowPage()
{
    FrontendMenuPage::ShowPage();
    SetPageButtons(ePageButtons_Cancel);
}

void FrontendMenuPage_SinglePlayer::PageCancelled()
{
    mFrontend.OnSinglePlayerCancelled();
}

void FrontendMenuPage_SinglePlayer::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    const char* menuItemId = sender->UserData().GetValue<const char*>();
    if (eventDesc.IsEvent(UiEventId_OnPress))
    {
        if (menuItemId == FrontendSinglePlayerPageSkirmish)
        {
            mFrontend.OnOpenSkirmishMenuSelected();
            return;
        }
    }
    FrontendMenuPage::HandleUiEvent(sender, eventDesc);
}

//////////////////////////////////////////////////////////////////////////

FrontendMenuPage_SkirmishMaps::FrontendMenuPage_SkirmishMaps(FrontendController& frontend)
    : FrontendMenuPage(frontend, eFrontendMenuPage_SkirmishMaps, "menu_page_4")
{
}

void FrontendMenuPage_SkirmishMaps::ConfigureMaps(cxx::span<ScenarioLevelInfo> mapsList)
{
    mMapsList.assign(mapsList.begin(), mapsList.end());

    ResetMapsScroll();
    ResetMapsSelectedItem();
}

void FrontendMenuPage_SkirmishMaps::ResetMapsScroll()
{
    mTableFirstItem = 0;
}

void FrontendMenuPage_SkirmishMaps::ResetMapsSelectedItem()
{
    mTableSelectedItem = 0;
}

void FrontendMenuPage_SkirmishMaps::SetMapsScrollPosition(int position, bool fromScrollBar)
{
    if ((position == mTableFirstItem) || mMapsList.empty())
        return;

    const int prevFirstItem = mTableFirstItem;

    int maxScrollPosition = static_cast<int>(mMapsList.size()) - static_cast<int>(mTableRows.size());
    if (maxScrollPosition < 0)
    {
        maxScrollPosition = 0;
    }

    mTableFirstItem = std::clamp(position, 0, maxScrollPosition);
    if (prevFirstItem == mTableFirstItem)
        return;

    RefreshTable();
    if (!fromScrollBar)
    {
        RefreshMapsScrollBar();
    }
}

void FrontendMenuPage_SkirmishMaps::SetSelectedItem(int itemIndex)
{
    if (itemIndex == mTableSelectedItem)
        return;

    mTableSelectedItem = itemIndex;
    RefreshTable();
    RefreshSelectionInfo();
}

bool FrontendMenuPage_SkirmishMaps::BindPageControls(UiHierarchy* hier)
{
    bool isSuccess = FrontendMenuPage::BindPageControls(hier);
    if (isSuccess)
    {
        UiWidget* tableGrid = mPageRoot->FindChildWithName("grid");
        cxx_assert(tableGrid);

        if (tableGrid)
        {
            tableGrid->Subscribe(this);
        }

        mMapsListScrollBar = (UiScrollBar*) mPageRoot->FindChildWithName("select_map_scrollbar");
        cxx_assert(mMapsListScrollBar);

        if (mMapsListScrollBar)
        {
            mMapsListScrollBar->Subscribe(this);
        }

        UiWidget* tableRow = mHierarchy->GetWidgetTemplate("skirmish_maps_table_row");
        cxx_assert(tableRow);

        // allocate rows
        for (int irow = 0, MaxRows = 8; irow < MaxRows; ++irow)
        {
            NewTableRow(tableGrid, tableRow);
        }

        mMapNameTextBox = (UiTextBox*) mHierarchy->GetWidgetByPath(mPageRoot, "map_info.title");
        cxx_assert(mMapNameTextBox);

        mMapPreviewPicture = (UiPicture*) mHierarchy->GetWidgetByPath(mPageRoot, "map_info.preview");
        cxx_assert(mMapPreviewPicture);
    }
    return isSuccess;
}

void FrontendMenuPage_SkirmishMaps::CleanupPageContent()
{
    FrontendMenuPage::CleanupPageContent();
    mMapsList.clear();
    mTableRows.clear();
    mTableFirstItem = 0;
    mTableSelectedItem = 0;
    mMapNameTextBox = nullptr;
    mMapPreviewPicture = nullptr;
    mMapsListScrollBar = nullptr;
}

void FrontendMenuPage_SkirmishMaps::ShowPage()
{
    FrontendMenuPage::ShowPage();
    SetPageButtons(ePageButtons_Both);

    RefreshTable();
    RefreshSelectionInfo();
    RefreshMapsScrollBar();
}

void FrontendMenuPage_SkirmishMaps::PageCancelled()
{
    mFrontend.OnSkirmishMapSelectCancelled();
}

void FrontendMenuPage_SkirmishMaps::PageConfirmed()
{
    if (mMapsList.empty())
    {
        cxx_assert(false);
        return;
    }

    mFrontend.OnSkirmishMapSelectConfirmed(mMapsList[mTableSelectedItem].mFileName);
}

void FrontendMenuPage_SkirmishMaps::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    FrontendMenuPage::HandleUiEvent(sender, eventDesc);

    if (eventDesc.IsEvent(UiEventId_OnPress))
    {
        int rowIndex = cxx::get_first_index_if(mTableRows, [sender](const TableRow& tableRow)
            {
                return (sender == tableRow.mWidget);
            });
        if (rowIndex != -1)
        {
            SetSelectedItem(mTableFirstItem + rowIndex);
        }
        return;
    }

    if (eventDesc.IsEvent(UiEventId_Wheel))
    {
        SetMapsScrollPosition(mTableFirstItem - eventDesc.mDelta.y, false);
        return;
    }

    if (eventDesc.IsEvent(UiEventId_ChangeScroll))
    {
        SetMapsScrollPosition(eventDesc.mScroll.y, true);
        return;
    }
}

void FrontendMenuPage_SkirmishMaps::RefreshTable()
{
    int rowsCounter = 0;
    int mapsCount = static_cast<int>(mMapsList.size());
    for (TableRow& roller: mTableRows)
    {
        const int itemIndex = mTableFirstItem + rowsCounter;
        ShowTableRow(rowsCounter, (itemIndex < mapsCount));
        if (itemIndex < mapsCount)
        {
            RefreshTableRow(rowsCounter, (itemIndex == mTableSelectedItem), mMapsList[itemIndex]);
        }
        ++rowsCounter;
    }
}

void FrontendMenuPage_SkirmishMaps::RefreshTableRow(int rowIndex, bool isSelected, const ScenarioLevelInfo& levelInfo)
{
    const TableRow& tableRow = mTableRows[rowIndex];
    if (UiTextBox* mapNameText = tableRow.mMapNameTextBox)
    {
        mapNameText->SetText(levelInfo.mLevelName);
    }
    if (UiTextBox* mapSizeText = tableRow.mMapSizeTextBox)
    {
        mapSizeText->SetText(cxx::va(L"%d x %d", levelInfo.mMapDimsX, levelInfo.mMapDimsY));
    }
    if (UiTextBox* maxPlayersText = tableRow.mMaxPlayersTextBox)
    {
        maxPlayersText->SetText(cxx::va(L"%d", levelInfo.mMaxPlayerCount));
    }
    if (UiWidget* selectionWidget = tableRow.mSelectionWidget)
    {
        selectionWidget->SetVisible(isSelected);
    }
}

void FrontendMenuPage_SkirmishMaps::RefreshSelectionInfo()
{
    if (mMapsList.empty())
        return;

    const ScenarioLevelInfo& levelInfo = mMapsList[mTableSelectedItem];
    if (mMapNameTextBox)
    {
        mMapNameTextBox->SetText(levelInfo.mLevelName);
    }

    if (mMapPreviewPicture)
    {
        std::string thumbnailPath = cxx::va("Data/editor/maps/Thumbnails/%s144.bmp", levelInfo.mFileName.c_str());
        if (!gFiles.PathToFileExists(thumbnailPath))
        {
            thumbnailPath = "black_back2"; // fallback
        }
        mMapPreviewPicture->SetPicture(thumbnailPath);
    }
}

void FrontendMenuPage_SkirmishMaps::RefreshMapsScrollBar()
{
    if (mMapsListScrollBar == nullptr)
        return;

    int maxScrollPosition = static_cast<int>(mMapsList.size()) - static_cast<int>(mTableRows.size());
    if (maxScrollPosition < 0)
    {
        maxScrollPosition = 0;
    }

    mMapsListScrollBar->SetScrollRange(0, maxScrollPosition);
    mMapsListScrollBar->SetScrollPosition(mTableFirstItem);
}

void FrontendMenuPage_SkirmishMaps::ShowTableRow(int rowIndex, bool isShown)
{
    if (UiWidget* rowWidget = mTableRows[rowIndex].mWidget)
    {
        rowWidget->SetVisible(isShown);
    }
}

void FrontendMenuPage_SkirmishMaps::NewTableRow(UiWidget* table, UiWidget* tableRowTemplate)
{
    cxx_assert(table);
    cxx_assert(tableRowTemplate);

    UiWidget* newInstance = tableRowTemplate->CloneWidget();
    cxx_assert(newInstance);

    if (newInstance == nullptr)
        return;

    table->AttachChild(newInstance);

    TableRow& tableRow = mTableRows.emplace_back();
    tableRow.mWidget = newInstance;

    tableRow.mMapNameTextBox = (UiTextBox*)newInstance->FindChildWithName("mapname");
    cxx_assert(tableRow.mMapNameTextBox);

    tableRow.mMaxPlayersTextBox = (UiTextBox*)newInstance->FindChildWithName("players");
    cxx_assert(tableRow.mMaxPlayersTextBox);

    tableRow.mMapSizeTextBox = (UiTextBox*)newInstance->FindChildWithName("mapsize");
    cxx_assert(tableRow.mMapSizeTextBox);

    tableRow.mSelectionWidget = newInstance->FindChildWithName("selected");
    cxx_assert(tableRow.mSelectionWidget);

    newInstance->Subscribe(this);
    newInstance->UserData().SetValue(newInstance);
}

//////////////////////////////////////////////////////////////////////////

FrontendMenuPage_MyPetDungeon::FrontendMenuPage_MyPetDungeon(FrontendController& frontend)
    : FrontendMenuPage(frontend, eFrontendMenuPage_MyPetDungeon, "menu_page_5")
{

}

void FrontendMenuPage_MyPetDungeon::ConfigureLevels(cxx::span<ScenarioLevelInfo> levelsList)
{
    mLevelsList.assign(levelsList.begin(), levelsList.end());
    RefreshLevelsList();
}

bool FrontendMenuPage_MyPetDungeon::BindPageControls(UiHierarchy* hier)
{
    bool isSuccess = FrontendMenuPage::BindPageControls(hier);
    if (isSuccess)
    {
        std::string textid;
        for (int icounter = 0; ; ++icounter)
        {
            cxx::str_printf(textid, "mpd%d", (icounter + 1));

            UiTextBox* textbox = (UiTextBox*) mPageRoot->FindChildWithName(textid);
            if (textbox == nullptr)
                break;

            textbox->UserData().SetParam0(FrontendActionTypeId_MyPetDungeonLevelSelect);
            textbox->UserData().SetParam1(icounter);
            textbox->Subscribe(this);

            mNamesList.push_back(textbox);
        }
    }
    return isSuccess;
}

void FrontendMenuPage_MyPetDungeon::CleanupPageContent()
{
    FrontendMenuPage::CleanupPageContent();
    mNamesList.clear();
}

void FrontendMenuPage_MyPetDungeon::ShowPage()
{
    FrontendMenuPage::ShowPage();
    SetPageButtons(ePageButtons_Cancel);
    RefreshLevelsList();
}

void FrontendMenuPage_MyPetDungeon::PageCancelled()
{
    mFrontend.OnMyPetDungeonMenuCancelled();
}

void FrontendMenuPage_MyPetDungeon::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    FrontendMenuPage::HandleUiEvent(sender, eventDesc);

    if (eventDesc.IsEvent(UiEventId_OnPress))
    {
        const FrontendActionTypeId actionId = sender->UserData().GetParam0<FrontendActionTypeId>();
        if (actionId == FrontendActionTypeId_MyPetDungeonLevelSelect)
        {
            const int ilevel = sender->UserData().GetParam1<int>();

            cxx_assert((ilevel > -1) && (ilevel < static_cast<int>(mLevelsList.size())));
            mFrontend.OnMyPetDungeonLevelSelect(mLevelsList[ilevel].mFileName);
            return;
        }
    }
}

void FrontendMenuPage_MyPetDungeon::RefreshLevelsList()
{
    const int MaxLevels = static_cast<int>(std::min(mNamesList.size(), mLevelsList.size()));

    int icounter = 0;
    for (UiTextBox* textbox: mNamesList)
    {
        bool isShown = (icounter < MaxLevels);
        if (isShown)
        {
            const ScenarioLevelInfo& levelInfo = mLevelsList[icounter];
            textbox->SetStringId(levelInfo.mBriefingTableId, 0);
        }
        textbox->SetVisible(isShown);
        ++icounter;
    }
}

//////////////////////////////////////////////////////////////////////////

FrontendMenuPage_MissionBriefing::FrontendMenuPage_MissionBriefing(FrontendController& frontend)
    : FrontendMenuPage(frontend, eFrontendMenuPage_MissionBriefing, "menu_page_6")
{

}

void FrontendMenuPage_MissionBriefing::ConfigureBriefing(const ScenarioLevelInfo& levelInfo)
{
    mLevelInfo = levelInfo;
    RefreshBriefing();
}

bool FrontendMenuPage_MissionBriefing::BindPageControls(UiHierarchy* hier)
{
    bool isSuccess = FrontendMenuPage::BindPageControls(hier);
    if (isSuccess)
    {
        mSubtitleTextBox = (UiTextBox*) mPageRoot->FindChildWithName("page_subtitle");
        cxx_assert(mSubtitleTextBox);

        mMainObjectiveTextBox = (UiTextBox*) mPageRoot->FindChildWithName("main_objective_text");
        cxx_assert(mMainObjectiveTextBox);

        mSubObjectiveTextBox = (UiTextBox*) mPageRoot->FindChildWithName("sub_objective_desc");
        cxx_assert(mSubObjectiveTextBox);

        if (UiPicture* pic = (UiPicture*) mPageRoot->FindChildWithName("obj_pic1"))
        {
            mObjectivePics.push_back(pic);
        }
        if (UiPicture* pic = (UiPicture*) mPageRoot->FindChildWithName("obj_pic2"))
        {
            mObjectivePics.push_back(pic);
        }
        cxx_assert(!mObjectivePics.empty());
    }
    return isSuccess;
}

void FrontendMenuPage_MissionBriefing::CleanupPageContent()
{
    FrontendMenuPage::CleanupPageContent();
    mSubtitleTextBox = nullptr;
    mMainObjectiveTextBox = nullptr;
    mSubObjectiveTextBox = nullptr;
    mObjectivePics.clear();
}

void FrontendMenuPage_MissionBriefing::ShowPage()
{
    FrontendMenuPage::ShowPage();
    SetPageButtons(ePageButtons_Both);
    ShowMainLogo(false);
    RefreshBriefing();
}

void FrontendMenuPage_MissionBriefing::PageCancelled()
{
    mFrontend.OnMissionBriefingCancelled(mLevelInfo.mFlags.mIsMyPetDungeonLevel);
}

void FrontendMenuPage_MissionBriefing::PageConfirmed()
{
    mFrontend.OnMissionBriefingConfirmed(mLevelInfo.mFileName);
}

void FrontendMenuPage_MissionBriefing::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    FrontendMenuPage::HandleUiEvent(sender, eventDesc);
}

void FrontendMenuPage_MissionBriefing::RefreshBriefing()
{
    if (mLevelInfo.mBriefingTableId != TextTableId_Null)
    {
        std::wstring textBuffer;
        if (mSubtitleTextBox)
        {
            std::wstring_view text1 = gTexts.GetString(mLevelInfo.mBriefingTableId, 0);
            std::wstring_view text2 = gTexts.GetString(mLevelInfo.mBriefingTableId, 1);
            cxx::str_wprintf(textBuffer, L"\"%.*ls\" - %.*ls", 
                static_cast<int>(text1.size()), text1.data(),
                static_cast<int>(text2.size()), text2.data());
            mSubtitleTextBox->SetText(textBuffer);
            textBuffer.clear();
        }

        if (mMainObjectiveTextBox)
        {
            std::wstring_view text = gTexts.GetString(mLevelInfo.mBriefingTableId, 2);
            cxx::str_wprintf(textBuffer, L"%.*ls", static_cast<int>(text.size()), text.data());
            mMainObjectiveTextBox->SetText(textBuffer);
            textBuffer.clear();
        }

        if (mSubObjectiveTextBox)
        {
            std::wstring_view text1 = gTexts.GetString(mLevelInfo.mBriefingTableId, 3);
            std::wstring_view text2 = gTexts.GetString(mLevelInfo.mBriefingTableId, 4);
            cxx::str_wprintf(textBuffer, L"-%.*ls\n-%.*ls", 
                static_cast<int>(text1.size()), text1.data(),
                static_cast<int>(text2.size()), text2.data());
            mSubObjectiveTextBox->SetText(textBuffer);
            textBuffer.clear();
        }
    }

    std::string picturePath;

    for (int i = 0; i < static_cast<int>(mObjectivePics.size()); ++i)
    {
        cxx::str_printf(picturePath, "%s-%d.png", mLevelInfo.mFileName.c_str(), i);
        if (gDK2AssetLoader.IsImageExists(picturePath))
        {
            mObjectivePics[i]->SetPicture(picturePath);
            mObjectivePics[i]->SetVisible(true);
        }
        else
        {
            mObjectivePics[i]->SetVisible(false);
        }
    }
}
