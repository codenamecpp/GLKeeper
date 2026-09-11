#include "stdafx.h"
#include "FrontendUiControls.h"
#include "UiWidget.h"
#include "FrontendController.h"
#include "UiTextBox.h"
#include "UiPicture.h"
#include "UiScrollBar.h"
#include "TextManager.h"
#include "DK2AssetLoader.h"

//////////////////////////////////////////////////////////////////////////

enum ActionTypeId : unsigned int
{
    ActionTypeId_None = 0,
    ActionTypeId_MyPetDungeonLevelSelect,
};

//////////////////////////////////////////////////////////////////////////

static const char* PagesCancel = "cancel";
static const char* PagesConfirm = "confirm";

//////////////////////////////////////////////////////////////////////////

FrontendUi::MenuPage::MenuPage(FrontendController& frontend, eMenuPage pageId, std::string_view pageRootName)
    : mPageId(pageId)
    , mFrontend(frontend)
    , mPageRootName(pageRootName)
{
}

bool FrontendUi::MenuPage::BindPageControls(UiHierarchy* hier)
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
        mPageCancelButton = mHierarchy->FindWidgetWithName(PagesCancel);
        if (mPageCancelButton)
        {
            mPageCancelButton->UserData().SetValue(PagesCancel);
        }
        cxx_assert(mPageCancelButton);

        mPageConfirmButton = mHierarchy->FindWidgetWithName(PagesConfirm);
        if (mPageConfirmButton)
        {
            mPageConfirmButton->UserData().SetValue(PagesConfirm);
        }
        cxx_assert(mPageConfirmButton);

        mMainLogo = mHierarchy->FindWidgetWithName("logo");
        cxx_assert(mMainLogo);
    }
    return true;
}

void FrontendUi::MenuPage::CleanupPageContent()
{
    SubscribePageButtons(false);
    mHierarchy = nullptr;
    mPageRoot = nullptr;
    mPageConfirmButton = nullptr;
    mPageCancelButton = nullptr;
    mMainLogo = nullptr;
}

void FrontendUi::MenuPage::ShowPage()
{
    if (mPageRoot && !mPageRoot->IsVisibleSelf())
    {
        mPageRoot->SetVisible(true);
        SetPageButtons(ePageButtons_None);
        ShowMainLogo(true);
        SubscribePageButtons(true);
    }
}

void FrontendUi::MenuPage::HidePage()
{
    if (mPageRoot && mPageRoot->IsVisibleSelf())
    {
        mPageRoot->SetVisible(false);
        SetPageButtons(ePageButtons_None);
        SubscribePageButtons(false);
    }
}

void FrontendUi::MenuPage::PageCancelled()
{
    cxx_assert(false);
}

void FrontendUi::MenuPage::PageConfirmed()
{
    cxx_assert(false);
}

void FrontendUi::MenuPage::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    const char* menuItemId = sender->UserData().GetValue<const char*>();
    if (eventDesc.IsEvent(UiEventId_OnPress))
    {
        if (menuItemId == PagesCancel)
        {
            PageCancelled();
            return;
        }

        if (menuItemId == PagesConfirm)
        {
            PageConfirmed();
            return;
        }
    }
}

void FrontendUi::MenuPage::SubscribePageButtons(bool isSubscribe)
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

void FrontendUi::MenuPage::SetPageButtons(ePageButtons pageButtons)
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

void FrontendUi::MenuPage::ShowMainLogo(bool showLogo)
{
    if (mMainLogo)
    {
        mMainLogo->SetVisible(showLogo);
    }
}

//////////////////////////////////////////////////////////////////////////

static const char* MainPageSinglePlayer = "single_player";
static const char* MainPageQuit = "quit";
static const char* MainPageMpd = "mpd";

//////////////////////////////////////////////////////////////////////////

FrontendUi::MenuPageMain::MenuPageMain(FrontendController& frontend)
    : MenuPage(frontend, eMenuPage_Main, "menu_page_1")
{
}

bool FrontendUi::MenuPageMain::BindPageControls(UiHierarchy* hier)
{
    bool isSuccess = MenuPage::BindPageControls(hier);
    if (isSuccess)
    {
        if (UiWidget* uiWidget = mPageRoot->FindChildWithName(MainPageSinglePlayer))
        {
            uiWidget->Subscribe(this);
            uiWidget->UserData().SetValue(MainPageSinglePlayer);
        }

        if (UiWidget* uiWidget = mPageRoot->FindChildWithName(MainPageQuit))
        {
            uiWidget->Subscribe(this);
            uiWidget->UserData().SetValue(MainPageQuit);
        }

        if (UiWidget* uiWidget = mPageRoot->FindChildWithName(MainPageMpd))
        {
            uiWidget->Subscribe(this);
            uiWidget->UserData().SetValue(MainPageMpd);
        }
    }
    return isSuccess;
}

void FrontendUi::MenuPageMain::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    const char* menuItemId = sender->UserData().GetValue<const char*>();
    if (eventDesc.IsEvent(UiEventId_OnPress))
    {
        if (menuItemId == MainPageSinglePlayer)
        {
            mFrontend.OnOpenSinglePlayerMenuSelected();
            return;
        }

        if (menuItemId == MainPageQuit)
        {
            mFrontend.OnQuitGameSelected();
            return;
        }

        if (menuItemId == MainPageMpd)
        {
            mFrontend.OnMyPetDungeonMenuSelected();
            return;
        }
    }
    MenuPage::HandleUiEvent(sender, eventDesc);
}

//////////////////////////////////////////////////////////////////////////

FrontendUi::MenuPageQuitGame::MenuPageQuitGame(FrontendController& frontend)
    : MenuPage(frontend, eMenuPage_QuitGame, "menu_page_3")
{
}

void FrontendUi::MenuPageQuitGame::ShowPage()
{
    MenuPage::ShowPage();
    SetPageButtons(ePageButtons_Both);
}

void FrontendUi::MenuPageQuitGame::PageCancelled()
{
    mFrontend.OnQuitGameCancelled();
}

void FrontendUi::MenuPageQuitGame::PageConfirmed()
{
    mFrontend.OnQuitGameConfirmed();
}

//////////////////////////////////////////////////////////////////////////

static const char* SinglePlayerPageSkirmish = "skirmish";

//////////////////////////////////////////////////////////////////////////

FrontendUi::MenuPageSinglePlayer::MenuPageSinglePlayer(FrontendController& frontend)
    : MenuPage(frontend, eMenuPage_SinglePlayer, "menu_page_2")
{
}

bool FrontendUi::MenuPageSinglePlayer::BindPageControls(UiHierarchy* hier)
{
    bool isSuccess = MenuPage::BindPageControls(hier);
    if (isSuccess)
    {
        if (UiWidget* uiWidget = mPageRoot->FindChildWithName(SinglePlayerPageSkirmish))
        {
            uiWidget->Subscribe(this);
            uiWidget->UserData().SetValue(SinglePlayerPageSkirmish);
        }
    }
    return isSuccess;
}

void FrontendUi::MenuPageSinglePlayer::ShowPage()
{
    MenuPage::ShowPage();
    SetPageButtons(ePageButtons_Cancel);
}

void FrontendUi::MenuPageSinglePlayer::PageCancelled()
{
    mFrontend.OnSinglePlayerCancelled();
}

void FrontendUi::MenuPageSinglePlayer::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    const char* menuItemId = sender->UserData().GetValue<const char*>();
    if (eventDesc.IsEvent(UiEventId_OnPress))
    {
        if (menuItemId == SinglePlayerPageSkirmish)
        {
            mFrontend.OnOpenSkirmishMenuSelected();
            return;
        }
    }
    MenuPage::HandleUiEvent(sender, eventDesc);
}

//////////////////////////////////////////////////////////////////////////

FrontendUi::MenuPageSkirmishMaps::MenuPageSkirmishMaps(FrontendController& frontend)
    : MenuPage(frontend, eMenuPage_SkirmishMaps, "menu_page_4")
{
}

void FrontendUi::MenuPageSkirmishMaps::ConfigureMaps(cxx::span<ScenarioLevelInfo> mapsList)
{
    mMapsList.assign(mapsList.begin(), mapsList.end());

    ResetMapsScroll();
    ResetMapsSelectedItem();
}

void FrontendUi::MenuPageSkirmishMaps::ResetMapsScroll()
{
    mTableFirstItem = 0;
}

void FrontendUi::MenuPageSkirmishMaps::ResetMapsSelectedItem()
{
    mTableSelectedItem = 0;
}

void FrontendUi::MenuPageSkirmishMaps::SetMapsScrollPosition(int position, bool fromScrollBar)
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

void FrontendUi::MenuPageSkirmishMaps::SetSelectedItem(int itemIndex)
{
    if (itemIndex == mTableSelectedItem)
        return;

    mTableSelectedItem = itemIndex;
    RefreshTable();
    RefreshSelectionInfo();
}

bool FrontendUi::MenuPageSkirmishMaps::BindPageControls(UiHierarchy* hier)
{
    bool isSuccess = MenuPage::BindPageControls(hier);
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

void FrontendUi::MenuPageSkirmishMaps::CleanupPageContent()
{
    MenuPage::CleanupPageContent();
    mMapsList.clear();
    mTableRows.clear();
    mTableFirstItem = 0;
    mTableSelectedItem = 0;
    mMapNameTextBox = nullptr;
    mMapPreviewPicture = nullptr;
    mMapsListScrollBar = nullptr;
}

void FrontendUi::MenuPageSkirmishMaps::ShowPage()
{
    MenuPage::ShowPage();
    SetPageButtons(ePageButtons_Both);

    RefreshTable();
    RefreshSelectionInfo();
    RefreshMapsScrollBar();
}

void FrontendUi::MenuPageSkirmishMaps::PageCancelled()
{
    mFrontend.OnSkirmishMapSelectCancelled();
}

void FrontendUi::MenuPageSkirmishMaps::PageConfirmed()
{
    if (mMapsList.empty())
    {
        cxx_assert(false);
        return;
    }

    mFrontend.OnSkirmishMapSelectConfirmed(mMapsList[mTableSelectedItem].mFileName);
}

void FrontendUi::MenuPageSkirmishMaps::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    MenuPage::HandleUiEvent(sender, eventDesc);

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

    if (eventDesc.IsEvent(UiEventId_OnMouseWheel))
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

void FrontendUi::MenuPageSkirmishMaps::RefreshTable()
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

void FrontendUi::MenuPageSkirmishMaps::RefreshTableRow(int rowIndex, bool isSelected, const ScenarioLevelInfo& levelInfo)
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

void FrontendUi::MenuPageSkirmishMaps::RefreshSelectionInfo()
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

void FrontendUi::MenuPageSkirmishMaps::RefreshMapsScrollBar()
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

void FrontendUi::MenuPageSkirmishMaps::ShowTableRow(int rowIndex, bool isShown)
{
    if (UiWidget* rowWidget = mTableRows[rowIndex].mWidget)
    {
        rowWidget->SetVisible(isShown);
    }
}

void FrontendUi::MenuPageSkirmishMaps::NewTableRow(UiWidget* table, UiWidget* tableRowTemplate)
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

FrontendUi::MenuPageMyPetDungeon::MenuPageMyPetDungeon(FrontendController& frontend)
    : MenuPage(frontend, eMenuPage_MyPetDungeon, "menu_page_5")
{

}

void FrontendUi::MenuPageMyPetDungeon::ConfigureLevels(cxx::span<ScenarioLevelInfo> levelsList)
{
    mLevelsList.assign(levelsList.begin(), levelsList.end());
    RefreshLevelsList();
}

bool FrontendUi::MenuPageMyPetDungeon::BindPageControls(UiHierarchy* hier)
{
    bool isSuccess = MenuPage::BindPageControls(hier);
    if (isSuccess)
    {
        std::string textid;
        for (int icounter = 0; ; ++icounter)
        {
            cxx::str_printf(textid, "mpd%d", (icounter + 1));

            UiTextBox* textbox = (UiTextBox*) mPageRoot->FindChildWithName(textid);
            if (textbox == nullptr)
                break;

            textbox->UserData().SetParam0(ActionTypeId_MyPetDungeonLevelSelect);
            textbox->UserData().SetParam1(icounter);
            textbox->Subscribe(this);

            mNamesList.push_back(textbox);
        }
    }
    return isSuccess;
}

void FrontendUi::MenuPageMyPetDungeon::CleanupPageContent()
{
    MenuPage::CleanupPageContent();
    mNamesList.clear();
}

void FrontendUi::MenuPageMyPetDungeon::ShowPage()
{
    MenuPage::ShowPage();
    SetPageButtons(ePageButtons_Cancel);
    RefreshLevelsList();
}

void FrontendUi::MenuPageMyPetDungeon::PageCancelled()
{
    mFrontend.OnMyPetDungeonMenuCancelled();
}

void FrontendUi::MenuPageMyPetDungeon::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    MenuPage::HandleUiEvent(sender, eventDesc);

    if (eventDesc.IsEvent(UiEventId_OnPress))
    {
        const ActionTypeId actionId = sender->UserData().GetParam0<ActionTypeId>();
        if (actionId == ActionTypeId_MyPetDungeonLevelSelect)
        {
            const int ilevel = sender->UserData().GetParam1<int>();

            cxx_assert((ilevel > -1) && (ilevel < static_cast<int>(mLevelsList.size())));
            mFrontend.OnMyPetDungeonLevelSelect(mLevelsList[ilevel].mFileName);
            return;
        }
    }
}

void FrontendUi::MenuPageMyPetDungeon::RefreshLevelsList()
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

FrontendUi::MenuPageMissionBriefing::MenuPageMissionBriefing(FrontendController& frontend)
    : MenuPage(frontend, eMenuPage_MissionBriefing, "menu_page_6")
{

}

void FrontendUi::MenuPageMissionBriefing::ConfigureBriefing(const ScenarioLevelInfo& levelInfo)
{
    mLevelInfo = levelInfo;
    RefreshBriefing();
}

bool FrontendUi::MenuPageMissionBriefing::BindPageControls(UiHierarchy* hier)
{
    bool isSuccess = MenuPage::BindPageControls(hier);
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

void FrontendUi::MenuPageMissionBriefing::CleanupPageContent()
{
    MenuPage::CleanupPageContent();
    mSubtitleTextBox = nullptr;
    mMainObjectiveTextBox = nullptr;
    mSubObjectiveTextBox = nullptr;
    mObjectivePics.clear();
}

void FrontendUi::MenuPageMissionBriefing::ShowPage()
{
    MenuPage::ShowPage();
    SetPageButtons(ePageButtons_Both);
    ShowMainLogo(false);
    RefreshBriefing();
}

void FrontendUi::MenuPageMissionBriefing::PageCancelled()
{
    mFrontend.OnMissionBriefingCancelled(mLevelInfo.mFlags.mIsMyPetDungeonLevel);
}

void FrontendUi::MenuPageMissionBriefing::PageConfirmed()
{
    mFrontend.OnMissionBriefingConfirmed(mLevelInfo.mFileName);
}

void FrontendUi::MenuPageMissionBriefing::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    MenuPage::HandleUiEvent(sender, eventDesc);
}

void FrontendUi::MenuPageMissionBriefing::RefreshBriefing()
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

//////////////////////////////////////////////////////////////////////////