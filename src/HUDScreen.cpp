#include "stdafx.h"
#include "HUDScreen.h"
#include "UiWidgetManager.h"
#include "UiHelpers.h"
#include "GameSession.h"
#include "UiTextBox.h"
#include "GameWorld.h"
#include "GameplayController.h"
#include "RoomManager.h"

//////////////////////////////////////////////////////////////////////////

static const std::string uiscreen_json_path = "ui/gameplay_screen.json";

// TODO: hardcoded
static const std::string KEEPERPANEL_ROOMS_BUTTON_HOVERED_FRAME = "GUI\\Icons\\frame";
static const std::string KEEPERPANEL_ROOMS_BUTTON_SELECTED_FRAME = "GUI\\Icons\\selected-room";
static const std::string KEEPERPANEL_SPELLS_BUTTON_HOVERED_FRAME = "GUI\\Icons\\frame";
static const std::string KEEPERPANEL_SPELLS_BUTTON_SELECTED_FRAME = "GUI\\Icons\\selected-spell";
static const std::string KEEPERPANEL_TRAPS_BUTTON_HOVERED_FRAME = "GUI\\Icons\\frame";
static const std::string KEEPERPANEL_TRAPS_BUTTON_SELECTED_FRAME = "GUI\\Icons\\selected-trap";

enum
{
    KEEPER_PANEL_BUTTON_SIZEW = 52,
    KEEPER_PANEL_BUTTON_SIZEH = 52,
};

//////////////////////////////////////////////////////////////////////////

HUDScreen::HUDScreen(GameplayController& gameplay)
    : mSellButton()
    , mDigButton()
    , mSelectedTab(UiKeeperTab_COUNT)
    , mGameplay(gameplay)
    , mDebugFPSLabel()
    , mDebugSceneStatsLabel()
{
    // reset buttons
    for (int iButton = 0; iButton < UiKeeperTab_COUNT; ++iButton)
    {
        mKeeperPanelTabs[iButton] = nullptr;
    }
    for (int iButton = 0; iButton < CountOf(mKeeperPanelButtons); ++iButton)
    {
        mKeeperPanelButtons[iButton] = nullptr;
    }

    mDebugStatsUpdateTimer.SetDuration(1.0f / 4.0f); // // times per second
}

void HUDScreen::InputEvent(KeyInputEvent& inputEvent)
{
    if (inputEvent.mPressed)
    {
        // reload screen
        if (inputEvent.mKeycode == KEYCODE_R && inputEvent.mMods == KEYMOD_CTRL)
        {
            inputEvent.SetConsumed();
            Deactivate();
            Cleanup();
            LoadContent();
            Activate();
            return;
        }
    }
}

void HUDScreen::UpdateFrame(float deltaTime)
{
    if (mDebugStatsUpdateTimer.TickAndCheckExpire(deltaTime))
    {
        UpdateDebugStatsText();
        mDebugStatsUpdateTimer.Start();
    }
}

void HUDScreen::UpdateDebugStatsText()
{
    if (mDebugSceneStatsLabel && mDebugSceneStatsLabel->IsVisibleInHierarchy())
    {
        const WorldStatistics& worldStats = gGameWorld.GetStatistics();

        int roomCount = gRoomManager.GetRoomCount();

        MapTile* hoveredTile = mGameplay.mHoveredTile;

        cxx::str_wprintf(mDebugStrBuffer, 
            L"Scene objects: %d\n"
            L"Rooms: %d\n"
            L".....................\n" 
                L"DIPs: %d\n" 
                L"Triangles count: %d\n" 
                L"IBO bind count: %d\n"
                L"VBO bind count: %d\n" 
                L"Programs bind count: %d\n" 
                L"Textures bind count: %d\n"
            L".....................\n" 
            L"Tile x: %d, y: %d\n"
            L"Tile Area: Land %d | Water %d | Lava %d"
            , worldStats.mNumSceneObjectsActive
            , roomCount
            , gRenderDevice.mFrameStats.mNumDIPs
            , gRenderDevice.mFrameStats.mNumTrianglesDrawn
            , gRenderDevice.mFrameStats.mNumSwitchIndexBuffers
            , gRenderDevice.mFrameStats.mNumSwitchVertexBuffers
            , gRenderDevice.mFrameStats.mNumSwitchPrograms
            , gRenderDevice.mFrameStats.mNumSwitchTextures
            , hoveredTile ? hoveredTile->mLocation.x : 0
            , hoveredTile ? hoveredTile->mLocation.y : 0
            , hoveredTile ? hoveredTile->mAreaCode[ePassabilityType_Land] : 0
            , hoveredTile ? hoveredTile->mAreaCode[ePassabilityType_Land_Water] : 0
            , hoveredTile ? hoveredTile->mAreaCode[ePassabilityType_Land_Any] : 0);
        mDebugSceneStatsLabel->SetText(mDebugStrBuffer);
    }

    if (mDebugFPSLabel && mDebugFPSLabel->IsVisibleInHierarchy())
    {
        cxx::str_wprintf(mDebugStrBuffer, L"FPS: %d | Min %d | Max %d", 
            gDebug.mFPSCounterLast, 
            gDebug.mFPSCounterMin, 
            gDebug.mFPSCounterMax);
        mDebugFPSLabel->SetText(mDebugStrBuffer);
    }

    mDebugStrBuffer.clear();
}

void HUDScreen::ShowDebugInfo(bool isShow)
{
    if (mDebugFPSLabel)
    {
        mDebugFPSLabel->SetVisible(isShow);
    }

    if (mDebugSceneStatsLabel)
    {
        mDebugSceneStatsLabel->SetVisible(isShow);
    }

    if (isShow)
    {
        UpdateDebugStatsText();
    }
}

bool HUDScreen::LoadContent()
{
    if (IsHierarchyLoaded())
        return true;

    if (!mHierarchy.LoadFrom(uiscreen_json_path))
    {
        cxx_assert(false);
        return false;
    }

    // bind controls
    bool isControlsBound = BindControls();
    cxx_assert(isControlsBound);
    if (!isControlsBound)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot bind screen controls %s", uiscreen_json_path.c_str());
    }

    return IsHierarchyLoaded();
}

bool HUDScreen::BindControls()
{
    mKeeperPanelTabs[UiKeeperTab_Creatures] = (UiProductionButton*) mHierarchy.GetWidgetByPath("tools_container.tabs_buttons.creatures_button");
    if (!mKeeperPanelTabs[UiKeeperTab_Creatures])
        return false;

    mKeeperPanelTabs[UiKeeperTab_Creatures]->Subscribe(this);
    mKeeperPanelTabs[UiKeeperTab_Creatures]->SetUnavailable(true);

    mKeeperPanelTabs[UiKeeperTab_Rooms] = (UiProductionButton*) mHierarchy.GetWidgetByPath("tools_container.tabs_buttons.rooms_button");
    if (!mKeeperPanelTabs[UiKeeperTab_Rooms]) 
        return false;

    mKeeperPanelTabs[UiKeeperTab_Rooms]->Subscribe(this);
    mKeeperPanelTabs[UiKeeperTab_Rooms]->SetUnavailable(false);

    mKeeperPanelTabs[UiKeeperTab_Spells] = (UiProductionButton*) mHierarchy.GetWidgetByPath("tools_container.tabs_buttons.spells_button");
    if (!mKeeperPanelTabs[UiKeeperTab_Spells])
        return false;

    mKeeperPanelTabs[UiKeeperTab_Spells]->Subscribe(this);
    mKeeperPanelTabs[UiKeeperTab_Spells]->SetUnavailable(true);

    mKeeperPanelTabs[UiKeeperTab_Traps] = (UiProductionButton*) mHierarchy.GetWidgetByPath("tools_container.tabs_buttons.traps_button");
    if (!mKeeperPanelTabs[UiKeeperTab_Traps])
        return false;

    mKeeperPanelTabs[UiKeeperTab_Traps]->Subscribe(this);
    mKeeperPanelTabs[UiKeeperTab_Traps]->SetUnavailable(true);

    mSellButton = (UiProductionButton*) mHierarchy.GetWidgetByPath("tools_container.tabs_buttons.sell_button");
    if (mSellButton)
    {
        mSellButton->Subscribe(this);
    }

    mDigButton = (UiProductionButton*) mHierarchy.GetWidgetByPath("tools_container.tabs_buttons.dig_button");
    if (mDigButton)
    {
        mDigButton->Subscribe(this);
    }

    // create production buttons
    UiGridLayout* gridLayout = (UiGridLayout*) mHierarchy.GetWidgetByPath("tools_container.production_buttons");
    cxx_assert(gridLayout);
    for (int iButton = 0; iButton < CountOf(mKeeperPanelButtons); ++iButton)
    {
        mKeeperPanelButtons[iButton] = gWidgetManager.ConstructWidget<UiProductionButton>();
        mKeeperPanelButtons[iButton]->SetVisible(false);
        mKeeperPanelButtons[iButton]->SetSize({KEEPER_PANEL_BUTTON_SIZEW, KEEPER_PANEL_BUTTON_SIZEH});
        mKeeperPanelButtons[iButton]->Subscribe(this);
        gridLayout->AttachChild(mKeeperPanelButtons[iButton]);
    }

    // money block
    mMoneyAmountTextbox = (UiTextBox*) mHierarchy.FindWidgetWithName("money_amount");
    if (mMoneyAmountTextbox)
    {
        mMoneyAmountTextbox->SetText(L"0");
    }

    // mana block
    mManaAmountTextbox = (UiTextBox*) mHierarchy.FindWidgetWithName("mana_amount");
    if (mManaAmountTextbox)
    {
        mManaAmountTextbox->SetText(L"0");
    }

    // debug info labels
    mDebugFPSLabel = (UiTextBox*) mHierarchy.GetWidgetByPath("fpslabel");
    mDebugSceneStatsLabel = (UiTextBox*) mHierarchy.GetWidgetByPath("scenestats");
    return true;
}

void HUDScreen::Cleanup()
{
    UiView::Cleanup();

    // reset buttons
    for (int iButton = 0; iButton < UiKeeperTab_COUNT; ++iButton)
    {
        mKeeperPanelTabs[iButton] = nullptr;
    }
    for (int iButton = 0; iButton < CountOf(mKeeperPanelButtons); ++iButton)
    {
        mKeeperPanelButtons[iButton] = nullptr;
    }
    mSellButton = nullptr;
    mDigButton = nullptr;
    mDebugFPSLabel = nullptr;
    mDebugSceneStatsLabel = nullptr;
    mMoneyAmountTextbox = nullptr;
    mManaAmountTextbox = nullptr;
    mSelectedTab = UiKeeperTab_COUNT;
}

void HUDScreen::OnActivated()
{
    SelectKeeperPanelTab(UiKeeperTab_Rooms);
    mDebugStatsUpdateTimer.Start();
    UpdateDebugStatsText();
}

void HUDScreen::OnDeactivated()
{}

void HUDScreen::SelectKeeperPanelTab(UiKeeperTab panelTab)
{
    if (mSelectedTab == panelTab)
        return;

    mSelectedTab = panelTab;
    OnKeeperPanelTabSelected(mSelectedTab);
}

void HUDScreen::OnKeeperPanelTabSelected(UiKeeperTab panelTab)
{
    int numButtonsUsed = 0;

    // init rooms buttons
    if (panelTab == UiKeeperTab_Rooms)
    {
        ScenarioDefinition& scenarioDefinitions = gGameSession.GetScenarioDefinition();

        const Player& localPlayer = gGameSession.GetLocalPlayer();

        cxx::temp_vector<RoomDefinition*> availableRooms;
        availableRooms.reserve(16);
        for (RoomDefinition& roomDef: scenarioDefinitions.mRoomDefs)
        {
            if (localPlayer.CanBuildRoomOfType(roomDef.mRoomType))
            {
                availableRooms.push_back(&roomDef);
            }
        }
        // sort in order
        std::sort(availableRooms.begin(), availableRooms.end(), [](RoomDefinition* LHS, RoomDefinition* RHS)
            {
                return LHS->mOrderInEditor < RHS->mOrderInEditor;
            });

        for (RoomDefinition* roomDef: availableRooms)
        {
            mKeeperPanelButtons[numButtonsUsed]->SetVisible(true);
            mKeeperPanelButtons[numButtonsUsed]->SetButtonStatePicture(eUiButtonState_Normal, roomDef->mGuiIcon.mResourceName);
            mKeeperPanelButtons[numButtonsUsed]->SetHoveredFramePicture(KEEPERPANEL_ROOMS_BUTTON_HOVERED_FRAME);
            mKeeperPanelButtons[numButtonsUsed]->SetSelectedFramePicture(KEEPERPANEL_ROOMS_BUTTON_SELECTED_FRAME);
            mKeeperPanelButtons[numButtonsUsed]->UserData().SetValue(roomDef);

            if (++numButtonsUsed == CountOf(mKeeperPanelButtons))
                break;
        }
    }

    // hide all unused buttons
    for (int iButton = numButtonsUsed; iButton < CountOf(mKeeperPanelButtons); ++iButton)
    {
        mKeeperPanelButtons[iButton]->SetVisible(false);
    }

    UpdateHUDState();
}

void HUDScreen::HandleUiEvent(UiWidget* eventSource, const UiEvent& eventDesc)
{
    bool acceptEvent = (eventSource && eventDesc.IsEvent(UiEventId_OnPress));
    if (!acceptEvent)
        return;

    // sell button
    if (eventSource == mSellButton)
    {
        if (mSellButton->IsSelected())
        {
            mGameplay.SetFreeInteraction();
        }
        else
        {
            mGameplay.SetRoomSellInteraction();
        }
        return;
    }

    if (eventSource == mDigButton)
    {
        if (mDigButton->IsSelected())
        {
            mGameplay.SetFreeInteraction();
        }
        else
        {
            mGameplay.SetDigTerrainInteraction();
        }
    }

    // tabs
    static const UiKeeperTab TabIndices[] =
    {
        UiKeeperTab_Creatures, 
        UiKeeperTab_Rooms, 
        UiKeeperTab_Spells, 
        UiKeeperTab_Traps,
    };
    for (UiKeeperTab tab: TabIndices)
    {
        if (mKeeperPanelTabs[tab] != eventSource)
            continue;

        SelectKeeperPanelTab(tab);
        return;
    }

    // test panel button pressed
    for (UiProductionButton* panelButton: mKeeperPanelButtons)
    {
        if (panelButton != eventSource)
            continue;

        OnKeeperPanelButtonClick(panelButton);
        return;
    }
}

void HUDScreen::OnKeeperPanelButtonClick(UiProductionButton* button)
{
    // room construction mode
    if (mSelectedTab == UiKeeperTab_Rooms)
    {
        RoomDefinition* roomDefinition = button->UserData().GetValue<RoomDefinition*>();
        mGameplay.SetRoomConstructionMode(roomDefinition);
    }
}

void HUDScreen::UpdateHUDState()
{
    // select keeper panel tab button
    for (UiProductionButton* button: mKeeperPanelTabs)
    {
        bool setSelected = (button == mKeeperPanelTabs[mSelectedTab]);
        if (button)
        {
            button->SetSelected(setSelected);
        }
    }

    // examine current interaction mode
    if (mGameplay.IsInInteractionMode(eMapInteractionMode_Free) || 
        mGameplay.IsInInteractionMode(eMapInteractionMode_Sell))
    {
        for (UiProductionButton* panelButton: mKeeperPanelButtons)
        {
            if (panelButton->IsSelected())
            {
                panelButton->SetSelected(false);
            }
        }
    }

    if (mSellButton)
    {
        mSellButton->SetSelected(mGameplay.IsInInteractionMode(eMapInteractionMode_Sell));
    }

    if (mDigButton)
    {
        mDigButton->SetSelected(mGameplay.IsInInteractionMode(eMapInteractionMode_Dig));
    }

    // room construction
    if (mGameplay.IsInInteractionMode(eMapInteractionMode_Build) && (mSelectedTab == UiKeeperTab_Rooms))
    {
        cxx_assert(mGameplay.mConstructRoomDef);
        for (UiProductionButton* panelButton: mKeeperPanelButtons)
        {
            bool isSelected = panelButton->UserData().GetValue<RoomDefinition*>() == mGameplay.mConstructRoomDef;
            panelButton->SetSelected(isSelected);
        }
    }

    UpdateMoneyInfo();
    UpdateManaInfo();
}

void HUDScreen::UpdateMoneyInfo()
{
    if (mMoneyAmountTextbox)
    {
        long moneyAmount = gGameSession.GetLocalPlayer().GetResourceAmount(eGameResource_Gold);
        mMoneyAmountTextbox->SetText(cxx::va(L"%ld", moneyAmount));
    }
}

void HUDScreen::UpdateManaInfo()
{
    if (mManaAmountTextbox)
    {
        long manaAmount = gGameSession.GetLocalPlayer().GetResourceAmount(eGameResource_Mana);
        mManaAmountTextbox->SetText(cxx::va(L"%ld", manaAmount));
    }
}
