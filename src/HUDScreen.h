#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiView.h"
#include "UiProductionButton.h"
#include "UiGridLayout.h"
#include "SimpleTimer.h"
#include "GameSessionDefs.h"

//////////////////////////////////////////////////////////////////////////

// Enum of Tabs on Keeper Panel
enum UiKeeperTab
{
    UiKeeperTab_Creatures,
    UiKeeperTab_Rooms,
    UiKeeperTab_Spells,
    UiKeeperTab_Traps,
    UiKeeperTab_COUNT
};

//////////////////////////////////////////////////////////////////////////
// Strategic mode screen
//////////////////////////////////////////////////////////////////////////

class HUDScreen final: public UiView, public UiEventListener
{
public:
    HUDScreen(GameplayController& gameplay);

    // override UiView
    bool LoadContent() override;
    void Cleanup() override;
    void InputEvent(KeyInputEvent& inputEvent) override;
    void UpdateFrame(float deltaTime) override;

    // Select hud panel tab and setup content
    // @param panelTab: Tab identifier
    void SelectKeeperPanelTab(UiKeeperTab panelTab);

    // Synchronize hud controls with current logic state
    void UpdateHUDState();
    void UpdateMoneyInfo();
    void UpdateManaInfo();

    // Enable or disable displaying fps counter
    // @param isShow: State
    void ShowDebugInfo(bool isShow);

private:
    void UpdateDebugStatsText();
    bool BindControls();
    void OnKeeperPanelTabSelected(UiKeeperTab panelTab);
    void OnKeeperPanelButtonClick(UiProductionButton* button);

    // override UiView
    void OnActivated() override;
    void OnDeactivated() override;

    // override UiEventListener
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;

private:
    UiKeeperTab mSelectedTab;
    UiProductionButton* mKeeperPanelTabs[UiKeeperTab_COUNT];
    UiProductionButton* mKeeperPanelButtons[16];
    UiProductionButton* mSellButton;
    UiProductionButton* mDigButton;
    UiTextBox* mDebugFPSLabel;
    UiTextBox* mDebugSceneStatsLabel;
    UiTextBox* mMoneyAmountTextbox = nullptr;
    UiTextBox* mManaAmountTextbox = nullptr;
    GameplayController& mGameplay;
    SimpleTimer mDebugStatsUpdateTimer;
    std::wstring mDebugStrBuffer;
};

//////////////////////////////////////////////////////////////////////////