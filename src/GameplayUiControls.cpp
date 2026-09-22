#include "stdafx.h"
#include "GameplayUiControls.h"
#include "UiHierarchy.h"
#include "GameplayController.h"
#include "UiRenderContext.h"
#include "UiButton.h"
#include "UiCompositeButton.h"
#include "GameMap.h"
#include "GameSession.h"
#include "Room.h"

//////////////////////////////////////////////////////////////////////////

static const StringHash ButtonSelectedStateNameHash = HashForString("selected");
static const StringHash ButtonAffordableStateNameHash = HashForString("affordable");

//////////////////////////////////////////////////////////////////////////

GameplayUi::Minimap::Minimap(GameplayController& gameplay)
    : mGameplay(gameplay)
{
    // MapColours.png

    mTerrainColors[eTerrainColors_Invalid] = Color32 {255, 0, 255, 255};
    mTerrainColors[eTerrainColors_TaggedForMining] = Color32 {0, 252, 156, 255};
    mTerrainColors[eTerrainColors_Water] = Color32 {20, 57, 62, 255};
    mTerrainColors[eTerrainColors_Gold] = Color32 {126, 109, 33, 255};
    mTerrainColors[eTerrainColors_Gems] = Color32 {110, 60, 102, 255};
    mTerrainColors[eTerrainColors_DirtPath] = Color32 {116, 100, 85, 255};
    mTerrainColors[eTerrainColors_Lava] = Color32 {107, 19, 6, 255};
    mTerrainColors[eTerrainColors_Rock] = Color32 {93, 73, 53, 255};
    mTerrainColors[eTerrainColors_ImpenetrableRock] = Color32 {0, 0, 0, 0};

    // reset
    for (auto& playersRoller: mPlayerColors)
    {
        for (Color32& colorsRoller: playersRoller)
        {
            colorsRoller = mTerrainColors[eTerrainColors_Invalid];
        }
    }

    mPlayerColors[ePlayerID_Keeper1][ePlayerColors_DungeonHeart] = Color32 {231, 24, 24, 255};
    mPlayerColors[ePlayerID_Keeper1][ePlayerColors_ClaimedPath] = Color32 {185, 0, 34, 255};
    mPlayerColors[ePlayerID_Keeper1][ePlayerColors_ReinforcedWall] = Color32 {142, 0, 0, 255};
    mPlayerColors[ePlayerID_Keeper1][ePlayerColors_Creature] = Color32 {142, 0, 0, 255};

    mPlayerColors[ePlayerID_Keeper2][ePlayerColors_DungeonHeart] = Color32 {50, 78, 230, 255};
    mPlayerColors[ePlayerID_Keeper2][ePlayerColors_ClaimedPath] = Color32 {46, 66, 175, 255};
    mPlayerColors[ePlayerID_Keeper2][ePlayerColors_ReinforcedWall] = Color32 {24, 41, 135, 255};
    mPlayerColors[ePlayerID_Keeper2][ePlayerColors_Creature] = Color32 {24, 41, 135, 255};

    mPlayerColors[ePlayerID_Keeper3][ePlayerColors_DungeonHeart] = Color32 {88, 191, 88, 255};
    mPlayerColors[ePlayerID_Keeper3][ePlayerColors_ClaimedPath] = Color32 {71, 168, 72, 255};
    mPlayerColors[ePlayerID_Keeper3][ePlayerColors_ReinforcedWall] = Color32 {18, 128, 19, 255};
    mPlayerColors[ePlayerID_Keeper3][ePlayerColors_Creature] = Color32 {18, 128, 19, 255};

    mPlayerColors[ePlayerID_Keeper4][ePlayerColors_DungeonHeart] = Color32 {230, 169, 50, 255};
    mPlayerColors[ePlayerID_Keeper4][ePlayerColors_ClaimedPath] = Color32 {190, 142, 50, 255};
    mPlayerColors[ePlayerID_Keeper4][ePlayerColors_ReinforcedWall] = Color32 {160, 105, 0, 255};
    mPlayerColors[ePlayerID_Keeper4][ePlayerColors_Creature] = Color32 {160, 105, 0, 255};

    mAnimCycleColors.push_back({142, 0, 0, 255});
    mAnimCycleColors.push_back({24, 41, 135, 255});
    mAnimCycleColors.push_back({18, 128, 19, 255});
    mAnimCycleColors.push_back({160, 105, 0, 255});
    mAnimCycleColors.push_back({174, 174, 174, 255});
    mAnimCycleColors.push_back({129, 129, 129, 255});
}

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

    // init texture
    mDimensions = gGameMap.GetDimensions();
    cxx_assert((mDimensions.x > 0) && (mDimensions.y > 0));
    if ((mDimensions.x > 0) && (mDimensions.y > 0))
    {
        const Point2D potDims
        {
            cxx::get_next_pot(mDimensions.x),
            cxx::get_next_pot(mDimensions.y)
        };
        if (!mScratchBuffer.Create(ePixelFormat_RGBA8, potDims, Color32{0, 0, 0, 0}))
        {
            cxx_assert(false);
            mDimensions = {};
        }
    }

    mAnimCycleTimer = {};
    mAnimCycleTimer.Start(1.0 / 4.0f);

    mTextureDirty = true;
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
    mTextureDirty = false;
    mTexture.reset();
    mScratchBuffer.Clear();
    mAnimCycleTimer = {};
    mAnimCycleColorIndex = {};
}

void GameplayUi::Minimap::UpdateFrame(float deltaTime)
{
    mGameplay.GetCameraInfo(mLastCameraInfo);

    // update anim cycle animation
    if (mAnimCycleTimer.IsOngoing() && 
        mAnimCycleTimer.TickAndCheckExpire(deltaTime))
    {
        mAnimCycleTimer.Start();
        mAnimCycleColorIndex = (mAnimCycleColorIndex + 1) % mAnimCycleColors.size();
        RequestUpdateMinimap();
    }
}

void GameplayUi::Minimap::UpdateColorData()
{
    if (!mScratchBuffer.HasContent())
        return;

    const ePlayerID localPlayerId = gGameSession.GetLocalPlayerId();

    Color32* pixels = (Color32*) mScratchBuffer.GetMipPixels(0);
    cxx_assert(pixels);

    const Point2D bufferDims = mScratchBuffer.GetDimensions();

    auto tilesIterator = gGameMap.IterateTiles();
    for (MapTile* mapTile = tilesIterator.NextTile(); mapTile; 
        mapTile = tilesIterator.NextTile())
    {
        TerrainDefinition* terrainDef = mapTile->GetTerrain();
        Color32 pixelColor {0, 0, 0, 0};
        switch (terrainDef->mTerrainType)
        {
            case TerrainTypeId_ImpenetrableRock: pixelColor = mTerrainColors[eTerrainColors_ImpenetrableRock]; break;
            case TerrainTypeId_Water: pixelColor = mTerrainColors[eTerrainColors_Water]; break;
            case TerrainTypeId_Lava: pixelColor = mTerrainColors[eTerrainColors_Lava]; break;
            case TerrainTypeId_Gold: pixelColor = mTerrainColors[eTerrainColors_Gold]; break;
            case TerrainTypeId_Gems: pixelColor = mTerrainColors[eTerrainColors_Gems]; break;
            case TerrainTypeId_DirtPath: 
            case TerrainTypeId_ManaVault:
                pixelColor = mTerrainColors[eTerrainColors_DirtPath]; break;
            case TerrainTypeId_Rock: pixelColor = mTerrainColors[eTerrainColors_Rock]; break;
            case TerrainTypeId_DungeonHeart: 
                pixelColor = mPlayerColors[mapTile->mOwnerId][ePlayerColors_DungeonHeart]; 
            break;
            case TerrainTypeId_ClaimedVault:
            case TerrainTypeId_ClaimedPath:
                pixelColor = mPlayerColors[mapTile->mOwnerId][ePlayerColors_ClaimedPath]; 
            break;
            case TerrainTypeId_ReinforcedWall:
                pixelColor = mPlayerColors[mapTile->mOwnerId][ePlayerColors_ReinforcedWall]; 
            break;
            default:
            {
                if (mapTile->mRoomInstance)
                {
                    const ePlayerID roomOwnerId = mapTile->mRoomInstance->GetOwnerId();
                    pixelColor = (roomOwnerId == ePlayerID_Neutral) ? 
                        mAnimCycleColors[mAnimCycleColorIndex] : 
                        mPlayerColors[mapTile->mOwnerId][ePlayerColors_ClaimedPath]; 
                }
            }
            break;
        }

        if (mapTile->IsTaggedForDigging(localPlayerId))
        {
            pixelColor = mTerrainColors[eTerrainColors_TaggedForMining];
        }
        pixels[mapTile->mLocation.y * bufferDims.x + mapTile->mLocation.x] = pixelColor;
    }
}

void GameplayUi::Minimap::UpdateTexture()
{
    UpdateColorData();
    if (!mScratchBuffer.HasContent())
        return;

    if (mTexture == nullptr)
    {
        mTexture = gRenderDevice.CreateTexture2D();
        cxx_assert(mTexture);
        if (!mTexture->Create(mScratchBuffer, eTextureFiltering_None))
        {
            cxx_assert(false);
            mScratchBuffer.Clear();
        }
        return;
    }

    mTexture->Upload(mScratchBuffer.GetMipPixels(0));
}

bool GameplayUi::Minimap::CustomDraw(const UiWidget& widget, UiRenderContext& uiRenderContext)
{
    const Rect2D localBounds = widget.GetLocalBounds();
    if (localBounds.Empty())
        return false;

    if (mTextureDirty)
    {
        mTextureDirty = false;
        UpdateTexture();
    }

    if (!mScratchBuffer.HasContent())
        return false;

    const Rect2D srcRect {0, 0, mDimensions.x, mDimensions.y};
    const Rect2D dstRect = FitAspectContain(srcRect, localBounds);

    Quad2D quad;
    quad.BuildTextureQuad(mTexture->GetDimensions(), srcRect, dstRect, COLOR_WHITE);
    quad.RotateAroundCenter(cxx::angle_t::from_degrees(mLastCameraInfo.mRotation));
    uiRenderContext.DrawQuads(mTexture.get(), &quad, 1);

    // draw camera bounds
    const glm::vec2 axisX = quad.mPoints[3].mPosition - quad.mPoints[0].mPosition; // 2d dir right
    const glm::vec2 axisY = quad.mPoints[1].mPosition - quad.mPoints[0].mPosition; // 2d dir down
    float cam_u = (mLastCameraInfo.mPosition.x / MAP_TILE_SIZE) / (mDimensions.x * 1.0f);
    float cam_v = (mLastCameraInfo.mPosition.y / MAP_TILE_SIZE) / (mDimensions.y * 1.0f);
    const glm::vec2 pointScreenPos = quad.mPoints[0].mPosition + axisX * cam_u + axisY * cam_v;
    const Point2D cameraLocation2 = pointScreenPos;

    Rect2D cameraBounds = Rect2D::FromPoints(cameraLocation2, cameraLocation2);
    cameraBounds.Inflate(Point2D(25, 15)); // todo: utilize actual zoom
    uiRenderContext.DrawRect(cameraBounds, COLOR_WHITE);

    return true;
}

void GameplayUi::Minimap::RequestUpdateMinimap()
{
    mTextureDirty = true;
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
    , mMinimap(gameplay)
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

void GameplayUi::ControlPanel::UpdateFrame(float deltaTime)
{
    mMinimap.UpdateFrame(deltaTime);
}

//////////////////////////////////////////////////////////////////////////
