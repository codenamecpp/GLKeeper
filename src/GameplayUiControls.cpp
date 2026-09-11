#include "stdafx.h"
#include "GameplayUiControls.h"
#include "UiHierarchy.h"
#include "GameplayController.h"
#include "UiRenderContext.h"
#include "UiButton.h"
#include "UiCompositeButton.h"

//////////////////////////////////////////////////////////////////////////

static const StringHash ButtonSelectedStateNameHash = HashForString("selected");
static const StringHash ButtonAffordableStateNameHash = HashForString("affordable");

//////////////////////////////////////////////////////////////////////////

bool GameplayUi::Minimap::BindControls(UiWidget* minimapRoot)
{
    mRootWidget = minimapRoot;
    cxx_assert(mRootWidget);

    if (mRootWidget)
    {
        mViewWidget = mRootWidget->FindChildWithName("mmview");
    }
    cxx_assert(mViewWidget);
    if (mViewWidget)
    {
        mViewWidget->SetCustomPainter(this);
    }

    return true;
}

void GameplayUi::Minimap::Cleanup()
{
    if (mViewWidget)
    {
        mViewWidget->SetCustomPainter(nullptr);
    }
    mRootWidget = nullptr;
    mViewWidget = nullptr;
}

bool GameplayUi::Minimap::CustomDraw(const UiWidget& widget, UiRenderContext& uiRenderContext)
{
    //uiRenderContext.DrawRect({1, 1, 90, 90}, COLOR_YELLOW);
    //return true;
    return false;
}

//////////////////////////////////////////////////////////////////////////

bool GameplayUi::MinimapButton::BindControls(UiWidget* buttonWidget)
{
    mButtonWidget = (UiCompositeButton*) buttonWidget;
    cxx_assert(mButtonWidget);
    if (mButtonWidget)
    {
        mButtonWidget->Subscribe(this);
        mButtonWidget->SetCustomState(ButtonSelectedStateNameHash, false);
    }
    return true;
}

void GameplayUi::MinimapButton::Cleanup()
{
    if (mButtonWidget)
    {
        mButtonWidget->Unsubscribe(this);
    }
    mButtonWidget = {};
    mOnPressCallback = {};
}

void GameplayUi::MinimapButton::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    if (sender != mButtonWidget)
    {
        cxx_assert(false);
        return;
    }

    if (eventDesc.IsEvent(UiEventId_OnPress))
    {
        if (mOnPressCallback)
        {
            mOnPressCallback();
        }
    }
}

void GameplayUi::MinimapButton::SetSelected(bool isSelected)
{
    if (mButtonWidget)
    {
        mButtonWidget->SetCustomState(ButtonSelectedStateNameHash, isSelected);
    }
}

void GameplayUi::MinimapButton::SetOnPressCallback(MinimapButtonCallback callback)
{
    mOnPressCallback = callback;
}

//////////////////////////////////////////////////////////////////////////

bool GameplayUi::SubjectButton::BindControls(UiWidget* buttonWidget)
{
    mButtonWidget = (UiCompositeButton*) buttonWidget;
    cxx_assert(mButtonWidget);

    if (mButtonWidget)
    {
        mButtonWidget->Subscribe(this);
        mButtonWidget->SetCustomState(ButtonSelectedStateNameHash, false);
        mButtonWidget->SetCustomState(ButtonAffordableStateNameHash, true);
    }
    return true;
}

void GameplayUi::SubjectButton::Cleanup()
{
    if (mButtonWidget)
    {
        mButtonWidget->Unsubscribe(this);
    }
    mButtonWidget = {};
    mOnPressCallback = {};
    mSubject = {};
}

void GameplayUi::SubjectButton::SetSelected(bool isSelected)
{
    if (mButtonWidget)
    {
        mButtonWidget->SetCustomState(ButtonSelectedStateNameHash, isSelected);
    }
}

void GameplayUi::SubjectButton::SetAffordable(bool isAffordable)
{
    if (mButtonWidget)
    {
        mButtonWidget->SetCustomState(ButtonAffordableStateNameHash, isAffordable);
    }
}

void GameplayUi::SubjectButton::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    if (sender != mButtonWidget)
    {
        cxx_assert(false);
        return;
    }

    if (eventDesc.IsEvent(UiEventId_OnPress))
    {
        if (mOnPressCallback)
        {
            bool isAlt = (eventDesc.mMouseButton == MBUTTON_RIGHT);
            mOnPressCallback(mSubject, isAlt);
        }
    }
}

void GameplayUi::SubjectButton::SetOnPressCallback(SubjectButtonCallback callback)
{
    mOnPressCallback = callback;
}

void GameplayUi::SubjectButton::SetButtonIcon(const std::string& iconPath)
{
    if (mButtonWidget)
    {
        mButtonWidget->SetButtonStatePicture(eUiButtonState_Normal, iconPath);
    }
}

void GameplayUi::SubjectButton::SetSubject(const SubjectInfo& subject)
{
    mSubject = subject;
}

void GameplayUi::SubjectButton::SetVisible(bool isVisible)
{
    if (mButtonWidget)
    {
        mButtonWidget->SetVisible(isVisible);
    }
}

//////////////////////////////////////////////////////////////////////////

GameplayUi::CPPage::CPPage(GameplayController& gameplay, eControlPanelPage pageId)
    : mPageId(pageId)
    , mGameplay(gameplay)
{
}

void GameplayUi::CPPage::ReConfigure(const SummaryInfo& summaryInfo)
{

}

bool GameplayUi::CPPage::BindControls(UiHierarchy* hier, UiWidget* pageWidget)
{
    mPageWidget = pageWidget;
    cxx_assert(mPageWidget);

    if (mPageWidget == nullptr)
        return false;

    mPageForwardWidget = mPageWidget->FindChildWithName("page_forward");
    if (mPageForwardWidget)
    {
        mPageForwardWidget->Subscribe(this);
    }
    mPageBackWidget = mPageWidget->FindChildWithName("page_back");
    if (mPageBackWidget)
    {
        mPageBackWidget->Subscribe(this);
    }
    EnablePageScrollButtons(true);

    mPageWidget->SetVisible(false);
    return true;
}

void GameplayUi::CPPage::Cleanup()
{
    mPageWidget = nullptr;
    if (mPageForwardWidget)
    {
        mPageForwardWidget->Unsubscribe(this);
        mPageForwardWidget = nullptr;
    }
    if (mPageBackWidget)
    {
        mPageBackWidget->Unsubscribe(this);
        mPageBackWidget = nullptr;
    }
}

void GameplayUi::CPPage::ShowPage()
{
    cxx_assert(mPageWidget);
    mPageWidget->SetVisible(true);
}

void GameplayUi::CPPage::HidePage()
{
    cxx_assert(mPageWidget);
    mPageWidget->SetVisible(false);
}

void GameplayUi::CPPage::SetAvailable(bool isAvailable)
{
    cxx_assert(mPageWidget);
    mPageWidget->SetEnabled(isAvailable);
}

void GameplayUi::CPPage::EnablePageScrollButtons(bool isEnabled)
{
    if (mPageBackWidget)
    {
        mPageBackWidget->SetEnabled(isEnabled);
    }
    if (mPageForwardWidget)
    {
        mPageForwardWidget->SetEnabled(isEnabled);
    }
}

//////////////////////////////////////////////////////////////////////////

GameplayUi::CPPageCreatures::CPPageCreatures(GameplayController& gameplay)
    : CPPage(gameplay, eControlPanelPage_Creatures)
{
}

void GameplayUi::CPPageCreatures::ReConfigure(const SummaryInfo& summaryInfo)
{
    CPPage::ReConfigure(summaryInfo);
}

bool GameplayUi::CPPageCreatures::BindControls(UiHierarchy* hier, UiWidget* pageWidget)
{
    bool isSuccess = CPPage::BindControls(hier, pageWidget);
    if (isSuccess)
    {

    }
    return isSuccess;
}

void GameplayUi::CPPageCreatures::Cleanup()
{
    CPPage::Cleanup();
}

void GameplayUi::CPPageCreatures::ShowPage()
{
    CPPage::ShowPage();
}

void GameplayUi::CPPageCreatures::HidePage()
{
    CPPage::HidePage();
}

//////////////////////////////////////////////////////////////////////////

GameplayUi::CPPageRooms::CPPageRooms(GameplayController& gameplay)
    : CPPage(gameplay, eControlPanelPage_Rooms)
{
}

void GameplayUi::CPPageRooms::ReConfigure(const SummaryInfo& summaryInfo)
{
    CPPage::ReConfigure(summaryInfo);
    
    mRoomEntries = summaryInfo.mRoomRecords;
    RefreshRoomButtons(true);
}

bool GameplayUi::CPPageRooms::BindControls(UiHierarchy* hier, UiWidget* pageWidget)
{
    bool isSuccess = CPPage::BindControls(hier, pageWidget);
    if (isSuccess)
    {
        mItemsContainer = (UiGridLayout*) pageWidget->FindChildWithName("items");
        cxx_assert(mItemsContainer);

        std::string itemButtonTemplateId;
        if (pageWidget->CustomProps().GetProperty("item_button_tid", itemButtonTemplateId))
        {
            mRoomButtonTemplate = hier->GetWidgetTemplate(itemButtonTemplateId);
        }
        cxx_assert(mRoomButtonTemplate);

        // allocate rooms buttons
        if (mRoomButtonTemplate && mItemsContainer)
        {
            cxx_assert(mRoomsButtons.empty());
            while (!mRoomsButtons.full())
            {
                mRoomsButtons.push_back({});

                UiWidget* buttonWidget = mRoomButtonTemplate->CloneWidget();
                mItemsContainer->AttachChild(buttonWidget);

                SubjectButton& roomButton = mRoomsButtons.back();
                if (!roomButton.BindControls(buttonWidget))
                {
                    cxx_assert(false);
                }
                roomButton.SetOnPressCallback([this](const SubjectInfo& subject, bool isAlt)
                    {
                        if (isAlt)
                        {
                            this->mGameplay.FocusOnNextOwnedRoom(subject.mRoomDefinition);
                        }
                        else
                        {
                            this->mGameplay.SetRoomConstructionMode(subject.mRoomDefinition);
                        }
                    });
            }
            cxx_assert(!mRoomsButtons.empty());
        }
    }
    return isSuccess;
}

void GameplayUi::CPPageRooms::Cleanup()
{
    CPPage::Cleanup();
    for (SubjectButton& roller: mRoomsButtons)
    {
        roller.Cleanup();
    }
    mRoomsButtons.clear();
    mRoomButtonTemplate = nullptr;
    mItemsContainer = nullptr;
    mSelectedRoom = nullptr;
    mRoomEntries.clear();
}

void GameplayUi::CPPageRooms::ShowPage()
{
    CPPage::ShowPage();
    RefreshRoomButtons(true);
}

void GameplayUi::CPPageRooms::HidePage()
{
    CPPage::HidePage();
}

void GameplayUi::CPPageRooms::SetRoomSelected(RoomDefinition* definition)
{
    if (mSelectedRoom == definition)
        return;

    RoomDefinition* prevSelectedRoom = mSelectedRoom;
    mSelectedRoom = definition;

    // deselect prev
    int buttonIndex;
    if (prevSelectedRoom && GetRoomButtonIndex(prevSelectedRoom, buttonIndex))
    {
        RefreshRoomButton(buttonIndex);
    }
    // select curr
    if (mSelectedRoom && GetRoomButtonIndex(mSelectedRoom, buttonIndex))
    {
        RefreshRoomButton(buttonIndex);
    }
}

void GameplayUi::CPPageRooms::SetRoomInfo(const RoomInfo& roomInfo)
{
    int buttonIndex;
    if (GetRoomButtonIndex(roomInfo.mDefinition, buttonIndex)) // update
    {
        mRoomEntries[buttonIndex] = roomInfo;
        RefreshRoomButton(buttonIndex);
    }
    else // add new
    {
        mRoomEntries.push_back(roomInfo);
        RefreshRoomButtons(true);
    }    
}

void GameplayUi::CPPageRooms::RefreshRoomButtons(bool needResort)
{
    if (needResort && !mRoomEntries.empty())
    {
        std::sort(mRoomEntries.begin(), mRoomEntries.end(), [](const RoomInfo& LHS, const RoomInfo& RHS)
            {
                return LHS.mDefinition->mOrderInEditor < RHS.mDefinition->mOrderInEditor;
            });
    }

    for (int i = 0; i < mRoomsButtons.size(); ++i)
    {
        RefreshRoomButton(i);
    }
}

void GameplayUi::CPPageRooms::RefreshRoomButton(int buttonIndex)
{
    const int RoomsCount = static_cast<int>(mRoomEntries.size());

    SubjectButton& roomButton = mRoomsButtons[buttonIndex];
    roomButton.SetVisible(buttonIndex < RoomsCount);
    if (buttonIndex < RoomsCount)
    {
        const RoomInfo& roomInfo = mRoomEntries[buttonIndex];
        roomButton.SetButtonIcon(roomInfo.mDefinition->mGuiIcon.mResourceName);

        SubjectInfo buttonSubject;
        buttonSubject.mSubjectKind = eSubjectKind_Room;
        buttonSubject.mRoomDefinition = roomInfo.mDefinition;
        roomButton.SetSubject(buttonSubject); 
        roomButton.SetSelected(roomInfo.mDefinition == mSelectedRoom);
        roomButton.SetAffordable(roomInfo.mAffordable);
    }
    else
    {
        roomButton.SetSubject({});
        roomButton.SetSelected(false);
        roomButton.SetAffordable(false);
    }
}

bool GameplayUi::CPPageRooms::GetRoomButtonIndex(RoomDefinition* definition, int& buttonIndex) const
{
    const int itemIndex = cxx::get_first_index_if(mRoomEntries, [definition](const RoomInfo& roller)
        {
            return roller.mDefinition == definition;
        });
    buttonIndex = itemIndex;
    return itemIndex != -1;
}

//////////////////////////////////////////////////////////////////////////

GameplayUi::ControlPanel::ControlPanel(GameplayController& gameplay)
    : mGameplay(gameplay)
    , mRoomsPage(gameplay)
    , mCreaturesPage(gameplay)
    , mSpellsPage(gameplay, eControlPanelPage_Spells)
    , mWorkshopPage(gameplay, eControlPanelPage_Workshop)
    , mPages()
{
    mPages[eControlPanelPage_Creatures] = &mCreaturesPage;
    mPages[eControlPanelPage_Rooms] = &mRoomsPage;
    mPages[eControlPanelPage_Spells] = &mSpellsPage;
    mPages[eControlPanelPage_Workshop] = &mWorkshopPage;
    for (CPPage* roller: mPages)
    {
        cxx_assert(roller);
    }
}

bool GameplayUi::ControlPanel::BindControls(UiHierarchy* hier)
{
    mHierarchy = hier;
    cxx_assert(mHierarchy);
    if (mHierarchy == nullptr)
        return false;

    mCpRootWidget = hier->FindWidgetWithName("control_panel");
    cxx_assert(mCpRootWidget);
    if (mCpRootWidget == nullptr)
        return false;

    UiWidget* minimapRootWidget = hier->FindWidgetWithName("minimap");
    cxx_assert(minimapRootWidget);

    if (minimapRootWidget)
    {
        mMmSellButton.BindControls(minimapRootWidget->FindChildWithName("sell_button"));
        mMmSellButton.SetOnPressCallback([this]()
            {
                this->mGameplay.SetRoomSellInteraction();
            });
        mMmSettingsButton.BindControls(minimapRootWidget->FindChildWithName("options_button"));
        mMmInfoButton.BindControls(minimapRootWidget->FindChildWithName("info_button"));
        mMmZoomButton.BindControls(minimapRootWidget->FindChildWithName("zoom_button"));
    }

    mMinimap.BindControls(minimapRootWidget);

    mPagesButtons[eControlPanelPage_Creatures].BindControls(mCpRootWidget->FindChildWithName("creatures_page_button"));
    mPagesButtons[eControlPanelPage_Creatures].SetOnPressCallback([this](const SubjectInfo& context, bool isAlt)
        {
            this->mGameplay.OnCreaturesPageSelected(isAlt);
        });
    mPagesButtons[eControlPanelPage_Rooms].BindControls(mCpRootWidget->FindChildWithName("rooms_page_button"));
    mPagesButtons[eControlPanelPage_Rooms].SetOnPressCallback([this](const SubjectInfo& context, bool isAlt)
        {
            this->mGameplay.OnRoomsPageSelected(isAlt);
        });
    mPagesButtons[eControlPanelPage_Spells].BindControls(mCpRootWidget->FindChildWithName("spells_page_button"));
    mPagesButtons[eControlPanelPage_Spells].SetOnPressCallback([this](const SubjectInfo& context, bool isAlt)
        {
            this->mGameplay.OnSpellsPageSelected(isAlt);
        });
    mPagesButtons[eControlPanelPage_Workshop].BindControls(mCpRootWidget->FindChildWithName("workshop_page_button"));
    mPagesButtons[eControlPanelPage_Workshop].SetOnPressCallback([this](const SubjectInfo& context, bool isAlt)
        {
            this->mGameplay.OnTrapsPageSelected(isAlt);
        });

    mMinCpButtonWidget = mCpRootWidget->FindChildWithName("minimize");
    cxx_assert(mMinCpButtonWidget);
    if (mMinCpButtonWidget)
    {
        mMinCpButtonWidget->Subscribe(this);
    }

    auto getPageWidget = [](UiWidget* rootWidget, eControlPanelPage pageId) -> UiWidget*
        {
            static const std::string pageWidgetNames[eControlPanelPage_COUNT] = {
                "creatures_page", "rooms_page", "spells_page", "workshop_page" };
            cxx_assert(pageId < eControlPanelPage_COUNT);
            return rootWidget->FindChildWithName(pageWidgetNames[pageId]);
        };

    for (CPPage* roller: mPages)
    {
        if (roller == nullptr)
            continue;

        if (!roller->BindControls(mHierarchy, getPageWidget(mCpRootWidget, roller->GetPageId())))
        {
            cxx_assert(false);
        }
    }

    return true;
}

void GameplayUi::ControlPanel::Cleanup()
{   
    for (SubjectButton& roller: mPagesButtons)
    {
        roller.Cleanup();
    }
    for (CPPage* roller: mPages)
    {
        roller->Cleanup();
    }
    mMinimap.Cleanup();
    mMmZoomButton.Cleanup();
    mMmInfoButton.Cleanup();
    mMmSellButton.Cleanup();
    mMmSettingsButton.Cleanup();
    mHierarchy = nullptr;
    mCpRootWidget = nullptr;
    mMinCpButtonWidget = nullptr;
    mIsMinimized = false;
}

void GameplayUi::ControlPanel::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    if ((sender == mMinCpButtonWidget) && eventDesc.IsEvent(UiEventId_OnPress))
    {
        SetMinimized(!mIsMinimized);
    }
}

void GameplayUi::ControlPanel::SetSellButtonSelected(bool isSelected)
{
    mMmSellButton.SetSelected(isSelected);
}

void GameplayUi::ControlPanel::SetPageSelected(eControlPanelPage pageId)
{
    SetMinimized(false);
    for (int iroller = 0; iroller < eControlPanelPage_COUNT; ++iroller)
    {
        mPagesButtons[iroller].SetSelected(iroller == pageId);
    }

    for (CPPage* roller: mPages)
    {
        (roller->GetPageId() == pageId) ? roller->ShowPage() : roller->HidePage(); 
    }
}

void GameplayUi::ControlPanel::SetPageAvailable(eControlPanelPage pageId, bool isAvailable)
{
    cxx_assert(pageId < eControlPanelPage_COUNT);
    mPagesButtons[pageId].SetAffordable(isAvailable);
}

void GameplayUi::ControlPanel::ReConfigure(const SummaryInfo& summaryInfo)
{
    for (CPPage* roller: mPages)
    {
        roller->ReConfigure(summaryInfo);
    }

    for (CPPage* roller: mPages)
    {
        const eControlPanelPage pageId = roller->GetPageId();
        mPagesButtons[pageId].SetVisible(!cxx::contains(summaryInfo.mHidePages, pageId));
        mPagesButtons[pageId].SetAffordable(!cxx::contains(summaryInfo.mUnavailablePages, pageId));
    }
}

void GameplayUi::ControlPanel::SetRoomSelected(RoomDefinition* definition)
{
    mRoomsPage.SetRoomSelected(definition);
}

void GameplayUi::ControlPanel::ReConfigureRoomInfo(const RoomInfo& roomInfo)
{
    mRoomsPage.SetRoomInfo(roomInfo);
}

void GameplayUi::ControlPanel::SetMinimized(bool isMinimized)
{
    if ((mIsMinimized == isMinimized) || (mCpRootWidget == nullptr))
        return;

    const int vertOffset = mCpRootWidget->CustomProps().GetPropertyOrDefault("minimized_vert_offset", 0);

    mIsMinimized = isMinimized;
    if (mIsMinimized)
    {
        mCpRootWidget->SetPosition(mCpRootWidget->GetPosition() + Point2D {0, vertOffset});
    }
    else
    {
        mCpRootWidget->SetPosition({}); // reset position
    }
    // force cursor to new position
    gRenderDevice.SetHwCursorScreenPosition(gInputs.GetMousePosition() + (Point2D {0, vertOffset} * (mIsMinimized ? 1 : -1)));
}

//////////////////////////////////////////////////////////////////////////
