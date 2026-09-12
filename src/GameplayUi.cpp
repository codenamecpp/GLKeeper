#include "stdafx.h"
#include "GameplayUi.h"
#include "UiWidgetManager.h"
#include "UiHelpers.h"
#include "GameSession.h"
#include "UiTextBox.h"
#include "GameWorld.h"
#include "GameplayController.h"
#include "RoomManager.h"
#include "GameplayUiControls.h"
#include "EconomyService.h"
#include "GameEventBus.h"

//////////////////////////////////////////////////////////////////////////

static const std::string uiscreen_json_path = "ui/gameplay_screen.json";

//////////////////////////////////////////////////////////////////////////

void GameplayUi::SummaryInfo::Clear()
{
    mRoomRecords.clear();
    mCreatureRecords.clear();
    mSpellRecords.clear();
    mWorkshopRecords.clear();
    mHidePages.clear();
    mUnavailablePages.clear();
}

//////////////////////////////////////////////////////////////////////////

GameplayUi::GameplayUi(GameplayController& gameplay)
    : mGameplay(gameplay)
    , mDebugFPSLabel()
    , mDebugSceneStatsLabel()
{
    mDebugStatsUpdateTimer.SetDuration(1.0f / 4.0f); // // times per second

    mControlPanel = std::make_unique<ControlPanel>(gameplay);
}

GameplayUi::~GameplayUi()
{
    mControlPanel.reset();
}

void GameplayUi::InputEvent(KeyInputEvent& inputEvent)
{
}

void GameplayUi::UpdateFrame(float deltaTime)
{
    if (mDebugStatsUpdateTimer.TickAndCheckExpire(deltaTime))
    {
        UpdateDebugStatsText();
        mDebugStatsUpdateTimer.Start();
    }

    mControlPanel->UpdateFrame(deltaTime);
}

void GameplayUi::UpdateDebugStatsText()
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

void GameplayUi::ShowDebugInfo(bool isShow)
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

bool GameplayUi::LoadContent()
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

bool GameplayUi::BindControls()
{
    if (!mControlPanel->BindControls(&mHierarchy))
    {
        cxx_assert(false);
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

    if (UiWidget* statusContainer = mHierarchy.FindWidgetWithName("status_container"))
    {
        mFocusOnDungeonHeartButton = statusContainer->FindChildWithName("focus_on_heart");
        if (mFocusOnDungeonHeartButton)
        {
            mFocusOnDungeonHeartButton->Subscribe(this);
        }
    }

    // debug info labels
    mDebugFPSLabel = (UiTextBox*) mHierarchy.GetWidgetByPath("fpslabel");
    mDebugSceneStatsLabel = (UiTextBox*) mHierarchy.GetWidgetByPath("scenestats");
    return true;
}

void GameplayUi::Cleanup()
{
    UiView::Cleanup();

    mControlPanel->Cleanup();

    mDebugFPSLabel = nullptr;
    mDebugSceneStatsLabel = nullptr;
    mMoneyAmountTextbox = nullptr;
    mManaAmountTextbox = nullptr;
    mFocusOnDungeonHeartButton = nullptr;

    mSummaryInfoCache.Clear();
}

void GameplayUi::OnActivated()
{
    mDebugStatsUpdateTimer.Start();
    UpdateDebugStatsText();

    gGameEventBus.Subscribe(eGameEvent_ResourceAmountChanged, this);
}

void GameplayUi::OnDeactivated()
{
    gGameEventBus.Unsubscribe(this);
}

void GameplayUi::HandleUiEvent(UiWidget* eventSource, const UiEvent& eventDesc)
{
    bool acceptEvent = (eventSource && eventDesc.IsEvent(UiEventId_OnPress));
    if (acceptEvent && (eventSource == mFocusOnDungeonHeartButton) && (eventDesc.mMouseButton == MBUTTON_RIGHT))
    {
        mGameplay.FocusOnNextOwnedRoom(RoomTypeId_DungeonHeart);
    }
}

void GameplayUi::UpdateHUDState()
{
    mControlPanel->SetSellButtonSelected(mGameplay.IsInInteractionMode(eMapInteractionMode_Sell));

    if (mGameplay.IsInInteractionMode(eMapInteractionMode_Free) || 
        mGameplay.IsInInteractionMode(eMapInteractionMode_Sell))
    {
        mControlPanel->SetRoomSelected(nullptr);
    }

    if (mGameplay.IsInInteractionMode(eMapInteractionMode_Build))
    {
        cxx_assert(mGameplay.mConstructRoomDef);
        mControlPanel->SetRoomSelected(mGameplay.mConstructRoomDef);
    }

    UpdateMoneyInfo();
    UpdateManaInfo();
}

void GameplayUi::UpdateMoneyInfo()
{
    if (mMoneyAmountTextbox)
    {
        long moneyAmount = gGameSession.GetLocalPlayer().GetResourceAmount(eGameResource_Gold);
        mMoneyAmountTextbox->SetText(cxx::va(L"%ld", moneyAmount));
    }
}

void GameplayUi::UpdateManaInfo()
{
    if (mManaAmountTextbox)
    {
        long manaAmount = gGameSession.GetLocalPlayer().GetResourceAmount(eGameResource_Mana);
        mManaAmountTextbox->SetText(cxx::va(L"%ld", manaAmount));
    }
}

void GameplayUi::SetPageAvailable(eControlPanelPage pageId, bool isAvailable)
{
    mControlPanel->SetPageAvailable(pageId, isAvailable);
}

void GameplayUi::ConfigureUi()
{
    BuildSummary(mSummaryInfoCache);

    mControlPanel->ReConfigure(mSummaryInfoCache);
}

void GameplayUi::SelectControlPanelPage(eControlPanelPage pageId)
{
    mControlPanel->SetPageSelected(pageId);
}

void GameplayUi::BuildSummary(SummaryInfo& summary) const
{
    summary.Clear();

    const Player& localPlayer = gGameSession.GetLocalPlayer();

    // rooms
    summary.mRoomRecords.reserve(16);

    ScenarioDefinition& scenarioDefinitions = gGameSession.GetScenarioDefinition();
    for (RoomDefinition& roomDef: scenarioDefinitions.mRoomDefs)
    {
        if (localPlayer.CanBuildRoomOfType(&roomDef))
        {
            RoomInfo& roomInfo = summary.mRoomRecords.emplace_back();
            roomInfo.mDefinition = &roomDef;
            roomInfo.mBuildable = true;

            const long buildingCost = gEconomyService.CalculateBuildingCost(localPlayer.GetPlayerId(), &roomDef, 1);
            roomInfo.mAffordable = gEconomyService.HasEnoughResources(localPlayer.GetPlayerId(), eGameResource_Gold, buildingCost);

            roomInfo.mOwnedCount = 0;
        }
    }

    // creatures
    summary.mCreatureRecords.reserve(16);
    // todo

    // hide pages

    // unavailable pages
    summary.mUnavailablePages.push_back(eControlPanelPage_Workshop);
}

void GameplayUi::HandleGameEvent(const GameEvent& eventData)
{
    if (eventData.IsEvent(eGameEvent_ResourceAmountChanged) && 
        gGameSession.IsLocalPlayerId(eventData.mPlayerId))
    {
        UpdateAffordability();
        return;
    }
}

void GameplayUi::UpdateAffordability()
{
    const Player& localPlayer = gGameSession.GetLocalPlayer();

    for (RoomInfo& roller: mSummaryInfoCache.mRoomRecords)
    {
        const long buildingCost = gEconomyService.CalculateBuildingCost(localPlayer.GetPlayerId(), roller.mDefinition, 1);
        bool isAffordable = gEconomyService.HasEnoughResources(localPlayer.GetPlayerId(), eGameResource_Gold, buildingCost);
        if (isAffordable != roller.mAffordable)
        {
            roller.mAffordable = isAffordable;
            mControlPanel->ReConfigureRoomInfo(roller);
        }
    }
}
