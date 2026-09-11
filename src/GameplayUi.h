#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiView.h"
#include "UiGridLayout.h"
#include "SimpleTimer.h"
#include "GameSessionDefs.h"
#include "GameplayDefs.h"
#include "GameEvent.h"

//////////////////////////////////////////////////////////////////////////
// Strategic mode screen
//////////////////////////////////////////////////////////////////////////

class GameplayUi final: public UiView
    , private UiEventListener
    , private GameEventListener
{
public:
    
    //////////////////////////////////////////////////////////////////////////

    enum eControlPanelPage
    {
        eControlPanelPage_Creatures,
        eControlPanelPage_Rooms,
        eControlPanelPage_Spells,
        eControlPanelPage_Workshop,
        eControlPanelPage_COUNT
    };

    //////////////////////////////////////////////////////////////////////////

private:

    //////////////////////////////////////////////////////////////////////////

    // CP prefix is for Control Panel

    class ControlPanel;
    class CPPage;
    class CPPageCreatures;
    class CPPageRooms;
    class SubjectButton;
    class Minimap;
    class MinimapButton;
    enum eSubjectKind
    {
        eSubjectKind_None,
        eSubjectKind_Creature,
        eSubjectKind_Room,
        eSubjectKind_Spell,
        eSubjectKind_Trap,
        eSubjectKind_Special,
        eSubjectKind_COUNT
    };
    struct SubjectInfo
    {
        eSubjectKind mSubjectKind {};
        RoomDefinition* mRoomDefinition {};
        EntityHandle mSubjectHandle {};
    };
    using SubjectButtonCallback = std::function<void(const SubjectInfo& subject, bool isAlt)>;
    using MinimapButtonCallback = std::function<void()>;

    //////////////////////////////////////////////////////////////////////////

    struct RoomInfo
    {
        RoomDefinition* mDefinition = nullptr;
        bool mBuildable = true; // can build and sell
        bool mAffordable = true; // enough money to build
        int mOwnedCount = 0;
    };

    struct CreatureInfo
    {
        CreatureDefinition* mDefinition = nullptr;
        int mOwnedCount = 0;
        // todo: count by job
        // todo: count by mood
    };

    struct SpellInfo
    {
        bool mAffordable = true; // enough mana to cast
        bool mResearched = false;
        bool mUpgraded = false;
        bool mResearching = false; // research/upgrade
        int mProgress = 0; // research/upgrade
    };

    struct WorkshopItemInfo
    {
        bool mAffordable = false; // enough money or mana to build
        bool mManufacturing = false; // work in progress
        int mOwnedCount = 0;
        int mProgress = 0;
    };

    struct SummaryInfo
    {
    public:
        void Clear();
    public:
        std::vector<RoomInfo> mRoomRecords;
        std::vector<CreatureInfo> mCreatureRecords;
        std::vector<SpellInfo> mSpellRecords;
        std::vector<WorkshopItemInfo> mWorkshopRecords;
        std::vector<eControlPanelPage> mHidePages;
        std::vector<eControlPanelPage> mUnavailablePages;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    GameplayUi(GameplayController& gameplay);
    ~GameplayUi();

    // override UiView
    bool LoadContent() override;
    void Cleanup() override;
    void InputEvent(KeyInputEvent& inputEvent) override;
    void UpdateFrame(float deltaTime) override;

    void ConfigureUi();

    void SelectControlPanelPage(eControlPanelPage pageId);

    void SetPageAvailable(eControlPanelPage pageId, bool isAvailable);

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

    void BuildSummary(SummaryInfo& summary) const;

    void UpdateAffordability();

    // override UiView
    void OnActivated() override;
    void OnDeactivated() override;

    // override UiEventListener
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;

    // override GameEventListener
    void HandleGameEvent(const GameEvent& eventData) override;

private:
    GameplayController& mGameplay;
    std::unique_ptr<ControlPanel> mControlPanel;
    UiTextBox* mDebugFPSLabel;
    UiTextBox* mDebugSceneStatsLabel;
    UiTextBox* mMoneyAmountTextbox = nullptr;
    UiTextBox* mManaAmountTextbox = nullptr;
    UiWidget* mFocusOnDungeonHeartButton = nullptr;
    SimpleTimer mDebugStatsUpdateTimer;
    std::wstring mDebugStrBuffer;

    SummaryInfo mSummaryInfoCache;
};

//////////////////////////////////////////////////////////////////////////