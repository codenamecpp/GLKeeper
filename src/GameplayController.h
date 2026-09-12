#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameSessionController.h"
#include "GameplayUi.h"
#include "GameplayCameraController.h"
#include "DebugToolsUi.h"
#include "GameplayDefs.h"
#include "GameEvent.h"
#include "InHandThingView.h"

//////////////////////////////////////////////////////////////////////////

class GameplayController: public GameSessionController
    , private GameEventListener
{
public:
    // public for convenience, don't change these fields directly
    MapTile* mHoveredTile; 
    RoomDefinition* mConstructRoomDef; // specified if mode eHandOfEvilMode_ConstructRoom
    GameObjectDefinition* mConstructTrapDef;  // specified if mode eHandOfEvilMode_ConstructTrap

public:
    GameplayController();

    // Change current map interaction mode
    void SetRoomConstructionMode(RoomDefinition* roomDefinition);
    void SetTrapConstructionMode(GameObjectDefinition* trapDefinition);
    void SetFreeInteraction();
    void SetRoomSellInteraction();

    void FocusOnNextOwnedRoom(RoomDefinition* roomDefinition);
    void FocusOnNextOwnedRoom(RoomTypeId roomTypeId);

    inline eMapInteractionMode GetMapInteractionMode() const { return mMapInteractionMode; }
    inline bool IsInInteractionMode(eMapInteractionMode mode) const
    {
        return mMapInteractionMode == mode;
    }

    inline void GetCameraInfo(GameplayCameraInfo& cameraInfo) const
    {
        return mGameplayCamera.GetCameraInfo(cameraInfo);
    }

    // override GameSessionController
    void OnSessionLoaded() override;
    void OnSessionStart() override;
    void OnSessionShutdown() override;
    void UpdateFrame(float deltaTime) override;
    void UpdateLogic(float stepDeltaTime) override;
    void InputEvent(MouseButtonInputEvent& inputEvent) override;
    void InputEvent(KeyInputEvent& inputEvent) override;
    void InputEvent(MouseMovedInputEvent& inputEvent) override;
    void InputEvent(MouseScrollInputEvent& inputEvent) override;

    // override GameEventListener
    void HandleGameEvent(const GameEvent& eventData) override;

public:
    // gameplayscreen notifications
    void OnRoomsPageSelected(bool isAlt);
    void OnCreaturesPageSelected(bool isAlt);
    void OnSpellsPageSelected(bool isAlt);
    void OnTrapsPageSelected(bool isAlt);

private:
    void UpdateHoveredTile();
    MapTile* ScanHoveredTile() const;
    void BeginMultitileSelection();
    void EndMultitileSelection(bool success);
    void OnMapSelectionChanged();
    bool NeedToShowSelection() const;
    bool CanMultitileSelect() const;
    bool IsMultitileSelectionStarted() const;
    bool GetMapSelectionArea(Rect2D& selectionArea) const;
    void OnInteractionModeChanged();   
    void HandleInteractionOnArea(const Rect2D& tilesArea);
    bool HandleTagForDigging(const Rect2D& tilesArea);
    
    void UpdateMapSelectionTint();
    void UpdateInHandThing();

    void UpdateHoveredEntity();
    void OnHoveredEntityChanged(EntityHandle prevEntity);

    void HandleHoveredEntityInteraction(bool alt);
    void HandleSingleTileInteraction(bool alt);

private:
    GameplayUi mGameplayUi;
    GameplayCameraController mGameplayCamera;
    DebugToolsUi mDebugToolsUi;

    eMapInteractionMode mMapInteractionMode;

    EntityHandle mHoveredEntity;
    EntityHandle mLastFocusedEntity;

    InHandThingView mInHandThingView;

    MapTile* mSelectionStartTile;
};