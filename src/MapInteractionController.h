#pragma once

#include "GameDefs.h"

// game world interactions controller class
class MapInteractionController: public cxx::noncopyable
{
public:
    // readonly
    eMapInteractionMode mCurrentMode = eMapInteractionMode_Free;
    MapTile* mHoveredTile = nullptr;
    MapTile* mSelectionStartTile = nullptr;

    // specified if mode eMapInteractionMode_ConstructRooms
    RoomDefinition* mConstructRoomDef = nullptr;

    // specified if mode eMapInteractionMode_ConstructTraps
    GameObjectDefinition* mConstructTrapDef = nullptr;

public:
    MapInteractionController();

    // process state frame
    void UpdateFrame();

    // reset controller state
    void ResetState();

    // change current interaction mode to room construction
    // @param roomDefinition: Specific room construction type
    void SetRoomsConstruction(RoomDefinition* roomDefinition);

    // change current interaction mode to trap construction
    // @param trapDefinition: Specific trap construction type
    void SetTrapsConstruction(GameObjectDefinition* trapDefinition);

    // change current interaction mode to default
    void SetFreeModeInteraction();

    // change current interaction mode to sell rooms and traps
    void SetSellRoomsInteraction();

    // change current interaction mode to claim or destroy terrain tiles
    void SetDigTerrainInteraction();

    // process input event
    // @param inputEvent: Event data
    void ProcessInputEvent(MouseButtonInputEvent& inputEvent);
    void ProcessInputEvent(MouseMovedInputEvent& inputEvent);
    void ProcessInputEvent(MouseScrollInputEvent& inputEvent);
    void ProcessInputEvent(KeyInputEvent& inputEvent);
    void ProcessInputEvent(KeyCharEvent& inputEvent);

private:
    void ScanHoveredTile();
    void BeginMultitileSelection();
    void EndMultitileSelection(bool success);
    void OnSelectionChanged();
    bool NeedToShowSelection() const;
    bool CanMultitileSelect() const;
    bool IsMultitileSelectionStarted() const;
    bool GetTerrainSelectionArea(Rectangle& area) const;
    void OnInteractionModeChanged();   
    void ProcessInteractionOnArea(const Rectangle& area);
    bool ProcessTagTerrain(const Rectangle& area);
    void ProcessSingleTileInteraction();
    void ProcessSingleTileInteractionAlt();
};