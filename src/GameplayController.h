#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameSessionController.h"
#include "HUDScreenView.h"
#include "WorldViewCameraController.h"
#include "DebugToolsUi.h"
#include "GameplayDefs.h"
#include "GameEvent.h"

//////////////////////////////////////////////////////////////////////////

class GameplayController: public GameSessionController
    , private GameEventListener
{
public:
    // public for convenience, don't change these fields directly
    eMapInteractionMode mInteraction;
    MapTile* mHoveredTile; 
    RoomDefinition* mConstructRoomDef; // specified if mode eHandOfEvilMode_ConstructRoom
    GameObjectDefinition* mConstructTrapDef;  // specified if mode eHandOfEvilMode_ConstructTrap

public:
    GameplayController();

    // Change current interaction mode to room construction
    // @param roomDefinition: Specific room construction type
    void SetRoomConstruction(RoomDefinition* roomDefinition);

    // Change current interaction mode to trap construction
    // @param trapDefinition: Specific trap construction type
    void SetTrapConstruction(GameObjectDefinition* trapDefinition);

    // Change current interaction mode to default
    void SetFreeInteraction();

    // Change current interaction mode to sell rooms and traps
    void SetRoomSellInteraction();

    // Change current interaction mode to claim or destroy terrain tiles
    void SetDigTerrainInteraction();

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

private:
    void ScanHoveredTile();
    void BeginMultitileSelection();
    void EndMultitileSelection(bool success);
    void OnSelectionChanged();
    bool NeedToShowSelection() const;
    bool CanMultitileSelect() const;
    bool IsMultitileSelectionStarted() const;
    bool GetTerrainSelectionArea(MapArea2D* selectionArea) const;
    void OnInteractionModeChanged();   
    void HandleInteractionOnArea(const MapArea2D& tilesArea);
    bool HandleTagTerrain(const MapArea2D& tilesArea);

    void HandleSingleTileInteraction();
    void HandleSingleTileInteractionAlt();

private:
    HUDScreenView mHUDScreen;
    WorldViewCameraController mWorldViewCamera;
    DebugToolsUi mDebugToolsUi;

    MapTile* mSelectionStartTile;
};