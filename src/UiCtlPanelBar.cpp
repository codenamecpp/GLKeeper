#include "pch.h"
#include "UiCtlPanelBar.h"
#include "UiCtlPanelIcon.h"

UiCtlPanelBar::UiCtlPanelBar(GuiWidget* control)
    : mControl(control)
{
    debug_assert(mControl);
    BindControls();
}

UiCtlPanelBar::~UiCtlPanelBar()
{
}

void UiCtlPanelBar::SelectPanel(eUiCtlPanelId panel)
{
    if (mCurrentPanel == panel)
        return;

    mCurrentPanel = panel;
    SetupCurrentPanelContent();
}

void UiCtlPanelBar::SetupCurrentPanelContent()
{
    //if (mCtlPanelIconsContainer == nullptr)
    //{
    //    debug_assert(false);
    //    return;
    //}

    //// hide all buttons
    //for (UiCtlPanelIcon& currButton: mCtlPanelIcons)
    //{
    //    currButton.mControl->SetVisible(false);
    //}

    //mCtlPanelIcons.clear();

    //// init rooms buttons
    //if (mCurrentCtlPanel == eUiCtlPanelId_Rooms)
    //{
    //    std::vector<RoomDefinition*> availableRooms;
    //    availableRooms.reserve(gGameWorld.mScenarioData.mRoomDefs.size());
    //    for (int iroom = 0, numRooms = gGameWorld.mScenarioData.mRoomDefs.size(); iroom < numRooms; ++iroom)
    //    {
    //        RoomDefinition* roomDef = &gGameWorld.mScenarioData.mRoomDefs[iroom];
    //        // todo : rooms availability to player
    //        if (roomDef->mBuildable)
    //        {
    //            availableRooms.push_back(roomDef);
    //        }
    //    }
    //    // sort in order
    //    std::sort(availableRooms.begin(), availableRooms.end(), 
    //        [](RoomDefinition* lhs, RoomDefinition* rhs)
    //    {
    //        return lhs->mOrderInEditor < rhs->mOrderInEditor;
    //    });

    //    mCtlPanelIcons.resize(availableRooms.size());
    //    // bind buttons
    //    for (size_t icurr = 0; icurr < availableRooms.size(); ++icurr)
    //    {
    //        GuiWidget* currControl = mCtlPanelIconsContainer->GetChild(icurr);
    //        if (currControl == nullptr)
    //        {
    //            currControl = mHier.CreateTemplateWidget(UiCtlPanelIconTemplateId);
    //            debug_assert(currControl);
    //            mCtlPanelIconsContainer->AttachChild(currControl);
    //        }
    //        debug_assert(currControl);
    //        currControl->SetVisible(true);

    //        RoomDefinition* currRoomDef = availableRooms[icurr];
    //        mCtlPanelIcons[icurr].SetControl(currControl);
    //        Subscribe(GuiEventId_OnPressStart, currControl);

    //        Texture2D* roomIcon = gTexturesManager.GetTexture2D(currRoomDef->mGuiIconResource.mResourceName);
    //        mCtlPanelIcons[icurr].SetPicture(roomIcon);
    //    }
    //}

    //// set overlay texture same as tab overlay
    //for (size_t icurr = 0; icurr < mCtlPanelIcons.size(); ++icurr)
    //{
    //    Texture2D* overlayTexture = mCtlPanelTabsIcons[mCurrentCtlPanel].mActiveOverlay;
    //    mCtlPanelIcons[icurr].SetActiveOverlay(overlayTexture);
    //}

    //UpdateUserInterfaceState();
}

void UiCtlPanelBar::UnusePanelIcons()
{
    for (UiCtlPanelIcon* currIcon: mPanelIcons)
    {
        currIcon->mControl->SetVisible(false);
    }
}

UiCtlPanelIcon* UiCtlPanelBar::UsePanelIcon()
{
    //UiCtlPanelIcon* newIcon = nullptr;
    //for (UiCtlPanelIcon* currIcon: mPanelIcons)
    //{
    //    if (!currIcon->mControl->IsVisible())
    //    {
    //        newIcon = currIcon;
    //        break;
    //    }
    //}
    //if (newIcon == nullptr)
    //{
    //    GuiWidget* control = mCtlPanelIconsContainer->GetChild(icurr);
    //    if (currControl == nullptr)
    //    {
    //        currControl = mHier.CreateTemplateWidget(UiCtlPanelIconTemplateId);
    //        debug_assert(currControl);
    //        mCtlPanelIconsContainer->AttachChild(currControl);
    //    }
    //    debug_assert(currControl);
    //    currControl->SetVisible(true);

    //    RoomDefinition* currRoomDef = availableRooms[icurr];
    //    mCtlPanelIcons[icurr].SetControl(currControl);
    //    Subscribe(GuiEventId_OnPressStart, currControl);

    //    Texture2D* roomIcon = gTexturesManager.GetTexture2D(currRoomDef->mGuiIconResource.mResourceName);
    //    mCtlPanelIcons[icurr].SetPicture(roomIcon);
    //}
    return nullptr;
}

bool UiCtlPanelBar::ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue)
{
    return false;
}

void UiCtlPanelBar::HandlePressStart(GuiWidget* sender, eMouseButton mbutton)
{
}

void UiCtlPanelBar::HandlePressEnd(GuiWidget* sender, eMouseButton mbutton)
{
}

void UiCtlPanelBar::BindControls()
{
    //mCtlPanelIconsContainer = mHier.SearchForWidget("control_panel_content");
    //debug_assert(mCtlPanelIconsContainer);

    //// bind tabs buttons
    //mCtlPanelTabsIcons.resize(eUiCtlPanelId_COUNT);
    //{
    //    GuiWidget* control = mHier.SearchForWidget("ctl_panel_tab_creatures");
    //    debug_assert(control);
    //    Subscribe(GuiEventId_OnPressStart, control);
    //    mCtlPanelTabsIcons[eUiCtlPanelId_Creatures] = new UiCtlPanelIcon(control);
    //    mCtlPanelTabsIcons[eUiCtlPanelId_Creatures]->SetActivatePanelAction(eUiCtlPanelId_Creatures);
    //}
    //{
    //    GuiWidget* control = mHier.SearchForWidget("ctl_panel_tab_rooms");
    //    debug_assert(control);
    //    Subscribe(GuiEventId_OnPressStart, control);
    //    mCtlPanelTabsIcons[eUiCtlPanelId_Rooms] = new UiCtlPanelIcon(control);
    //    mCtlPanelTabsIcons[eUiCtlPanelId_Rooms]->SetActivatePanelAction(eUiCtlPanelId_Rooms);
    //}
    //{
    //    GuiWidget* control = mHier.SearchForWidget("ctl_panel_tab_spells");
    //    debug_assert(control);
    //    Subscribe(GuiEventId_OnPressStart, control);
    //    mCtlPanelTabsIcons[eUiCtlPanelId_Spells] = new UiCtlPanelIcon(control);
    //    mCtlPanelTabsIcons[eUiCtlPanelId_Spells]->SetActivatePanelAction(eUiCtlPanelId_Spells);
    //}
    //{
    //    GuiWidget* control = mHier.SearchForWidget("ctl_panel_tab_traps");
    //    debug_assert(control);
    //    Subscribe(GuiEventId_OnPressStart, control);
    //    mCtlPanelTabsIcons[eUiCtlPanelId_Traps] = new UiCtlPanelIcon(control);
    //    mCtlPanelTabsIcons[eUiCtlPanelId_Traps]->SetActivatePanelAction(eUiCtlPanelId_Traps);
    //}

    //// bind sell and dig buttons
    //{
    //    GuiWidget* control = mHier.SearchForWidget("ctl_panel_sell_button");
    //    debug_assert(control);
    //    Subscribe(GuiEventId_OnPressStart, control);
    //    mCtlPanelIconSell = new UiCtlPanelIcon(control);
    //    mCtlPanelIconSell->SetActivateSellAction();
    //}
    //{
    //    GuiWidget* control = mHier.SearchForWidget("ctl_panel_dig_button");
    //    debug_assert(control);
    //    Subscribe(GuiEventId_OnPressStart, control);
    //    mCtlPanelIconDig = new UiCtlPanelIcon(control);
    //    mCtlPanelIconDig->SetActivateDigAction();
    //}
}
