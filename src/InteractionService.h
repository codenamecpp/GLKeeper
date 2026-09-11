#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"

//////////////////////////////////////////////////////////////////////////

class InteractionService final: public cxx::noncopyable
{
public:
    void EnterWorld();
    void ClearWorld();

    // picks up the entity into the player's hand
    // checks game rules (world position and ownership) to ensure the entity can be picked up
    // on success, transfers the entity from the game world to the player's hand
    bool CanPickUpEntity(EntityHandle entityHandle, ePlayerID playerId) const;
    bool TryPickUpEntity(EntityHandle entityHandle, ePlayerID playerId);

    // drops the entity from the player's hand
    // checks game rules (world position and placement conditions) to ensure the entity can be dropped
    // on success, transfers the entity from the player's hand to the game world
    bool CanDropEntityOn(EntityHandle entityHandle, ePlayerID playerId, const glm::vec2& position) const;
    bool TryDropEntityOn(EntityHandle entityHandle, ePlayerID playerId, const glm::vec2& position);

    // builds rooms
    // checks game rules, building costs, and availability to ensure the entire room can be constructed
    // on success, constructs the room, transfers ownership to the player, withdraws the building cost from the player's gold reserves
    bool CanBuildRoom(ePlayerID playerId, RoomDefinition* roomDefinition, const Rect2D& mapArea, long& outBuildingCost, 
        cxx::any_vector<MapTile*> outRoomTiles) const;
    bool TryBuildRoom(ePlayerID playerId, RoomDefinition* roomDefinition, const Rect2D& mapArea, long& outBuildingCost, 
        cxx::any_vector<MapTile*> outRoomTiles);

    // sells rooms
    // checks game rules to ensure the entity can be sold
    // on success, deposits gold into the player's reserves
    bool CanSellRoom(ePlayerID playerId, const Rect2D& mapArea, long& outMoney, cxx::any_vector<MapTile*> outRoomTiles) const;
    bool TrySellRoom(ePlayerID playerId, const Rect2D& mapArea, long& outMoney, cxx::any_vector<MapTile*> outRoomTiles);

private:

};

//////////////////////////////////////////////////////////////////////////

extern InteractionService gInteractionService;

//////////////////////////////////////////////////////////////////////////