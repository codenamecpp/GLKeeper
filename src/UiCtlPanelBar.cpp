#include "pch.h"
#include "UiCtlPanelBar.h"
#include "UiCtlPanelIcon.h"
#include "GameWorld.h"
#include "GameMain.h"

UiCtlPanelBar::UiCtlPanelBar(GuiWidget* control)
    : mControl(control)
{
    debug_assert(mControl);
    BindControls();
    UnusePanelIcons();
}

UiCtlPanelBar::~UiCtlPanelBar()
{
    Cleanup();
}

void UiCtlPanelBar::SelectPanel(eUiCtlPanelId panel)
{
    if (mCurrentPanel == panel)
        return;

    mCurrentPanel = panel;
    SetupCurrentPanelContent();
}

void UiCtlPanelBar::UpdateUserInterfaceState()
{
    // tabs selection
    mIconCreatures->SetActiveState(mCurrentPanel == eUiCtlPanelId_Creatures);
    mIconRooms->SetActiveState(mCurrentPanel == eUiCtlPanelId_Rooms);
    mIconTraps->SetActiveState(mCurrentPanel == eUiCtlPanelId_Traps);
    mIconSpells->SetActiveState(mCurrentPanel == eUiCtlPanelId_Spells);

    MapInteractionController& mapInteraction = gGameMain.mGameplayGamestate.mMapInteractionControl;

    // sell & dig
    mIconSell->SetActiveState(mapInteraction.mCurrentMode == eMapInteractionMode_SellRooms);
    mIconDig->SetActiveState(mapInteraction.mCurrentMode == eMapInteractionMode_DigTerrain);

    // panel icons
    int numButtons = GetUsedPanelIconCount();
    for (int icurr = 0; icurr < numButtons; ++icurr)
    {
        UiCtlPanelIcon* currIcon = mPanelIcons[icurr];

        bool isActiveState = false;
        if (mapInteraction.mCurrentMode == eMapInteractionMode_ConstructRooms && mCurrentPanel == eUiCtlPanelId_Rooms)
        {
            debug_assert(mapInteraction.mConstructRoomDef);
            isActiveState = (mapInteraction.mConstructRoomDef == currIcon->mRoomDefinition);
        }
        if (mapInteraction.mCurrentMode == eMapInteractionMode_ConstructTraps && mCurrentPanel == eUiCtlPanelId_Traps)
        {
            debug_assert(mapInteraction.mConstructTrapDef);
            isActiveState = (mapInteraction.mConstructTrapDef == currIcon->mObjectDefinition);
        }
        currIcon->SetActiveState(isActiveState);
    }
}

void UiCtlPanelBar::SetupCurrentPanelContent()
{
    if (mCurrentPanel == eUiCtlPanelId_COUNT)
        return;

    // setup rooms buttons
    ScenarioData& scenario = gGameWorld.mScenarioData;
    if (mCurrentPanel == eUiCtlPanelId_Rooms)
    {
        std::vector<RoomDefinition*> availableRooms;
        availableRooms.reserve(scenario.mRoomDefs.size());
        for (RoomDefinition& currRoomDefinition: scenario.mRoomDefs)
        {
            // todo : rooms availability to player

            if (currRoomDefinition.mBuildable)
            {
                availableRooms.push_back(&currRoomDefinition);
            }
        }
        // sort in order
        std::sort(availableRooms.begin(), availableRooms.end(), [](RoomDefinition* lhs, RoomDefinition* rhs)
            {
                return lhs->mOrderInEditor < rhs->mOrderInEditor;
            });

        UsePanelIcons(availableRooms.size());
        // set icons
        for (size_t icurr = 0; icurr < availableRooms.size(); ++icurr)
        {
            RoomDefinition* currRoomDef = availableRooms[icurr];
            mPanelIcons[icurr]->SetContext(currRoomDef);
        }
    }
    else
    {
        UnusePanelIcons();
    }

    SetupCurrentPanelIconsOverlay();
    UpdateUserInterfaceState();
}

void UiCtlPanelBar::SetupCurrentPanelIconsOverlay()
{
    // set overlay texture same as tab overlay
    Texture2D* overlayTexture = nullptr;
    switch (mCurrentPanel)
    {
    case eUiCtlPanelId_Creatures: overlayTexture = mIconCreatures->mActiveOverlay;
        break;
    case eUiCtlPanelId_Rooms: overlayTexture = mIconRooms->mActiveOverlay;
        break;
    case eUiCtlPanelId_Spells: overlayTexture = mIconSpells->mActiveOverlay;
        break;
    case eUiCtlPanelId_Traps: overlayTexture = mIconTraps->mActiveOverlay;
        break;
    }
    debug_assert(overlayTexture);

    int numIcons = GetUsedPanelIconCount();
    for (int icurr = 0; icurr < numIcons; ++icurr)
    {
        UiCtlPanelIcon* currIcon = mPanelIcons[icurr];
        currIcon->SetActiveOverlay(overlayTexture);
    }
}

void UiCtlPanelBar::UsePanelIcons(int numIcons)
{
    int currentIconsCount = (int) mPanelIcons.size();

    if (currentIconsCount == 0)
    {
        debug_assert(false);
        return;
    }

    debug_assert(mPanelIconsContainer);

    while (currentIconsCount < numIcons)
    {
        ++currentIconsCount;

        // create new icons
        GuiWidget* controlClone = mPanelIcons[0]->mControl->CloneDeep();
        debug_assert(controlClone);
        mPanelIconsContainer->AttachChild(controlClone);
        Subscribe(GuiEventId_OnPressStart, controlClone);

        UiCtlPanelIcon* icon = new UiCtlPanelIcon(controlClone);
        mPanelIcons.push_back(icon);
    }

    for (int icurr = 0; icurr < currentIconsCount; ++icurr)
    {
        // show if used and hide otherwise
        mPanelIcons[icurr]->mControl->SetVisible(icurr < numIcons);
    }

    mPanelIconsContainer->UpdateLayout();
}

void UiCtlPanelBar::UnusePanelIcons()
{
    for (UiCtlPanelIcon* currIcon: mPanelIcons)
    {
        currIcon->mControl->SetVisible(false);
    }

    mPanelIconsContainer->UpdateLayout();
}

int UiCtlPanelBar::GetUsedPanelIconCount() const
{
    int counter = 0;
    for (UiCtlPanelIcon* currIcon: mPanelIcons)
    {
        if (!currIcon->mControl->IsVisible())
            break;

        ++counter;
    }
    return counter;
}

void UiCtlPanelBar::OnPressPanelIcon(UiCtlPanelIcon* panelIcon, bool isPrimaryAction)
{
    debug_assert(panelIcon);
    if (mCurrentPanel == eUiCtlPanelId_Rooms)
    {
        if (isPrimaryAction)
        {
            debug_assert(panelIcon->mRoomDefinition);
            gGameMain.mGameplayGamestate.mMapInteractionControl.SetRoomsConstruction(panelIcon->mRoomDefinition);
        }
    }
}

void UiCtlPanelBar::OnPressSellIcon()
{
    MapInteractionController& mapInteraction = gGameMain.mGameplayGamestate.mMapInteractionControl;
    if (mapInteraction.mCurrentMode == eMapInteractionMode_SellRooms)
    {
        mapInteraction.SetFreeModeInteraction();
    }
    else
    {
        mapInteraction.SetSellRoomsInteraction();
    }
}

void UiCtlPanelBar::OnPressDigIcon()
{
    MapInteractionController& mapInteraction = gGameMain.mGameplayGamestate.mMapInteractionControl;
    if (mapInteraction.mCurrentMode == eMapInteractionMode_DigTerrain)
    {
        mapInteraction.SetFreeModeInteraction();
    }
    else
    {
        mapInteraction.SetDigTerrainInteraction();
    }
}

bool UiCtlPanelBar::ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue)
{
    return false;
}

void UiCtlPanelBar::HandlePressStart(GuiWidget* sender, eMouseButton mbutton)
{
    bool primaryAction = (mbutton == eMouseButton_Left);
    bool secondaryAction = (mbutton == eMouseButton_Right);

    if (sender == mIconCreatures->mControl)
    {
        if (primaryAction)
        {
            SelectPanel(eUiCtlPanelId_Creatures);
        }
        return;
    }
    if (sender == mIconRooms->mControl)
    {
        if (primaryAction)
        {
            SelectPanel(eUiCtlPanelId_Rooms);
        }
        return;
    }
    if (sender == mIconTraps->mControl)
    {
        if (primaryAction)
        {
            SelectPanel(eUiCtlPanelId_Traps);
        }
        return;
    }
    if (sender == mIconSpells->mControl)
    {
        if (primaryAction)
        {
            SelectPanel(eUiCtlPanelId_Spells);
        }
        return;
    }
    if (sender == mIconSell->mControl)
    {
        if (primaryAction)
        {
            OnPressSellIcon();
        }
        return;
    }
    if (sender == mIconDig->mControl)
    {
        if (primaryAction)
        {
            OnPressDigIcon();
        }
        return;
    }
    // check panel icons
    for (UiCtlPanelIcon* currIcon : mPanelIcons)
    {
        if (sender == currIcon->mControl)
        {
            OnPressPanelIcon(currIcon, primaryAction);
            return;
        }
    }
}

void UiCtlPanelBar::HandlePressEnd(GuiWidget* sender, eMouseButton mbutton)
{
}

void UiCtlPanelBar::Cleanup()
{
    mPanelIconsContainer = nullptr;
    mCurrentPanel = eUiCtlPanelId_COUNT;
    SafeDelete(mIconCreatures);
    SafeDelete(mIconRooms);
    SafeDelete(mIconTraps);
    SafeDelete(mIconSpells);
    SafeDelete(mIconSell);
    SafeDelete(mIconDig);
    for(UiCtlPanelIcon* currIcon: mPanelIcons)
    {
        SafeDelete(currIcon);
    }
    mPanelIcons.clear();
}

void UiCtlPanelBar::BindControls()
{
    // bind tabs buttons
    {
        GuiWidget* control = mControl->SearchForChild("ctl_panel_tab_creatures");
        debug_assert(control);
        Subscribe(GuiEventId_OnPressStart, control);
        mIconCreatures = new UiCtlPanelIcon(control);
    }
    {
        GuiWidget* control = mControl->SearchForChild("ctl_panel_tab_rooms");
        debug_assert(control);
        Subscribe(GuiEventId_OnPressStart, control);
        mIconRooms = new UiCtlPanelIcon(control);
    }
    {
        GuiWidget* control = mControl->SearchForChild("ctl_panel_tab_traps");
        debug_assert(control);
        Subscribe(GuiEventId_OnPressStart, control);
        mIconTraps = new UiCtlPanelIcon(control);
    }
    {
        GuiWidget* control = mControl->SearchForChild("ctl_panel_tab_spells");
        debug_assert(control);
        Subscribe(GuiEventId_OnPressStart, control);
        mIconSpells = new UiCtlPanelIcon(control);
    }
    {
        GuiWidget* control = mControl->SearchForChild("ctl_panel_sell_button");
        debug_assert(control);
        Subscribe(GuiEventId_OnPressStart, control);
        mIconSell = new UiCtlPanelIcon(control);
    }
    {
        GuiWidget* control = mControl->SearchForChild("ctl_panel_dig_button");
        debug_assert(control);
        Subscribe(GuiEventId_OnPressStart, control);
        mIconDig = new UiCtlPanelIcon(control);
    }

    // bind icons
    mPanelIconsContainer = mControl->SearchForChild("control_panel_content");
    debug_assert(mPanelIconsContainer);

    if (mPanelIconsContainer)
    {
        for (GuiWidget* currControl = mPanelIconsContainer->GetChild(); currControl;
            currControl = currControl->NextSibling())
        {
            Subscribe(GuiEventId_OnPressStart, currControl);

            UiCtlPanelIcon* panelIcon = new UiCtlPanelIcon(currControl);
            mPanelIcons.push_back(panelIcon);
        }
    }
}
