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

    bool TestPickUpEntity(EntityHandle entityHandle, ePlayerID playerId) const;
    bool PickUpEntity(EntityHandle entityHandle, ePlayerID playerId);

    // drops the entity from the player's hand
    // checks game rules (world position and placement conditions) to ensure the entity can be dropped
    // on success, transfers the entity from the player's hand to the game world

    bool TestDropEntityOn(EntityHandle entityHandle, ePlayerID playerId, const glm::vec2& position) const;
    bool DropEntityOn(EntityHandle entityHandle, ePlayerID playerId, const glm::vec2& position);

private:
};

//////////////////////////////////////////////////////////////////////////

extern InteractionService gInteractionService;

//////////////////////////////////////////////////////////////////////////