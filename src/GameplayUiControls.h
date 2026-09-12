#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "UiDefs.h"
#include "UiEvent.h"
#include "GameSessionDefs.h"
#include "GameplayDefs.h"
#include "GameplayUi.h"
#include "UiPainter.h"

//////////////////////////////////////////////////////////////////////////

class GameplayUi::Minimap final: public UiEventListener, public UiPainter
{
private:
    enum eTerrainColors
    {
        eTerrainColors_Invalid,
        eTerrainColors_TaggedForMining,
        eTerrainColors_Water,
        eTerrainColors_Lava,
        eTerrainColors_Gold,
        eTerrainColors_Gems,
        eTerrainColors_DirtPath,
        eTerrainColors_Rock,
        eTerrainColors_ImpenetrableRock,
        eTerrainColors_COUNT
    };
    enum ePlayerColors
    {
        ePlayerColors_DungeonHeart,
        ePlayerColors_ClaimedPath,
        ePlayerColors_ReinforcedWall,
        ePlayerColors_Creature,
        ePlayerColors_COUNT
    };
public:
    Minimap(GameplayController& gameplay);
    bool BindControls(UiWidget* minimapRoot);
    void Cleanup();
    void UpdateFrame(float deltaTime);
    void RequestUpdateMinimap();
    // override UiPainter
    bool CustomDraw(const UiWidget& widget, UiRenderContext& uiRenderContext) override;
private:
    void UpdateColorData();
    void UpdateTexture();
private:
    GameplayController& mGameplay;
    Color32 mPlayerColors[ePlayerID_COUNT][ePlayerColors_COUNT];
    Color32 mTerrainColors[eTerrainColors_COUNT];
    UiWidget* mRootWidget {};
    UiWidget* mViewWidget {};
    Point2D mDimensions {};
    GameplayCameraInfo mLastCameraInfo;
    std::unique_ptr<GpuTexture2D> mTexture;
    BitmapImage mScratchBuffer;
    cxx::static_vector<Color32, 6> mAnimCycleColors;
    SimpleTimer mAnimCycleTimer {};
    int mAnimCycleColorIndex = 0;
    bool mTextureDirty = false;
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
    
class GameplayUi::MinimapButton final: public UiEventListener
{
public:
    bool BindControls(UiWidget* buttonWidget);
    void Cleanup();
    void SetOnPressCallback(MinimapButtonCallback callback);
    void SetSelected(bool isSelected);
    // override UiEventListener
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;
private:
    MinimapButtonCallback mOnPressCallback;
    UiCompositeButton* mButtonWidget {};
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class GameplayUi::SubjectButton final: public UiEventListener
{
public:
    bool BindControls(UiWidget* buttonWidget);
    void Cleanup();
    void SetOnPressCallback(SubjectButtonCallback callback);
    void SetVisible(bool isVisible);
    void SetSelected(bool isSelected);
    void SetAffordable(bool isAffordable);
    void SetButtonIcon(const std::string& iconPath);
    void SetSubject(const SubjectInfo& subject);
    // override UiEventListener
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;
private:
    SubjectButtonCallback mOnPressCallback;
    SubjectInfo mSubject;
    UiCompositeButton* mButtonWidget {};
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class GameplayUi::CPPage: public UiEventListener
{
    const eControlPanelPage mPageId;
public:
    CPPage(GameplayController& gameplay, eControlPanelPage pageId);
    // overridables
    inline eControlPanelPage GetPageId() const { return mPageId; }
    virtual void ReConfigure(const SummaryInfo& summaryInfo);
    virtual bool BindControls(UiHierarchy* hier, UiWidget* pageWidget);
    virtual void Cleanup();
    virtual void ShowPage();
    virtual void HidePage();
    virtual void SetAvailable(bool isAvailable);
    void EnablePageScrollButtons(bool isEnabled);
protected:
    GameplayController& mGameplay;
    UiWidget* mPageWidget {};
    UiWidget* mPageForwardWidget {};
    UiWidget* mPageBackWidget {};
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class GameplayUi::CPPageCreatures: public GameplayUi::CPPage
{
public:
    CPPageCreatures(GameplayController& gameplay);
    // override ControlPanelPage
    void ReConfigure(const SummaryInfo& summaryInfo) override;
    bool BindControls(UiHierarchy* hier, UiWidget* pageWidget) override;
    void Cleanup() override;
    void ShowPage() override;
    void HidePage() override;
private:
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class GameplayUi::CPPageRooms: public GameplayUi::CPPage
{
public:
    CPPageRooms(GameplayController& gameplay);
    // override ControlPanelPage
    void ReConfigure(const SummaryInfo& summaryInfo) override;
    bool BindControls(UiHierarchy* hier, UiWidget* pageWidget) override;
    void Cleanup() override;
    void ShowPage() override;
    void HidePage() override;
    void SetRoomSelected(RoomDefinition* definition);
    void SetRoomInfo(const RoomInfo& roomInfo);
private:
    void RefreshRoomButtons(bool needResort);
    void RefreshRoomButton(int buttonIndex);
    bool GetRoomButtonIndex(RoomDefinition* definition, int& buttonIndex) const;
private:
    UiWidget* mRoomButtonTemplate = nullptr;
    UiGridLayout* mItemsContainer = nullptr;
    RoomDefinition* mSelectedRoom = nullptr;
    cxx::static_vector<SubjectButton, 20> mRoomsButtons;
    std::vector<RoomInfo> mRoomEntries;
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class GameplayUi::ControlPanel final: public UiEventListener
{
public:
    ControlPanel(GameplayController& gameplay);
    void ReConfigure(const SummaryInfo& summaryInfo);
    bool BindControls(UiHierarchy* hier);
    void UpdateFrame(float deltaTime);
    void Cleanup();
    void SetMinimized(bool isMinimized);
    void SetSellButtonSelected(bool isSelected);
    void SetRoomSelected(RoomDefinition* definition);
    void ReConfigureRoomInfo(const RoomInfo& roomInfo);
    void SetPageSelected(eControlPanelPage pageId);
    void SetPageAvailable(eControlPanelPage pageId, bool isAvailable);
    // override UiEventListener
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;
private:
    GameplayController& mGameplay;
    UiHierarchy* mHierarchy {};
    UiWidget* mCpRootWidget {};
    UiWidget* mMinCpButtonWidget {};
    Minimap mMinimap;
    // minimap buttons
    MinimapButton mMmSellButton;
    MinimapButton mMmSettingsButton;
    MinimapButton mMmInfoButton;
    MinimapButton mMmZoomButton;
    // pages
    SubjectButton mPagesButtons[eControlPanelPage_COUNT];
    CPPageCreatures mCreaturesPage;
    CPPageRooms mRoomsPage;
    CPPage mSpellsPage;
    CPPage mWorkshopPage;
    CPPage* mPages[eControlPanelPage_COUNT];
    // extra
    bool mIsMinimized = false;
};

//////////////////////////////////////////////////////////////////////////

