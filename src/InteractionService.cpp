#include "stdafx.h"
#include "InteractionService.h"
#include "CreatureManager.h"
#include "GameSession.h"
#include "MapUtils.h"
#include "GameMap.h"

//////////////////////////////////////////////////////////////////////////

InteractionService gInteractionService;

//////////////////////////////////////////////////////////////////////////

void InteractionService::EnterWorld()
{

}

void InteractionService::ClearWorld()
{

}

bool InteractionService::TestPickUpEntity(EntityHandle entityHandle, ePlayerID playerId) const
{
    if (!gGameSession.GetPlayer(playerId).IsAlive())
        return false;

    // handle pickup creature
    if (entityHandle.IsCreature())
    {
        Creature* creature = gCreatureManager.GetCreaturePtr(entityHandle);
        if ((creature == nullptr) || !creature->ExistsOnMap())
            return false;

        if (!creature->HasOwner(ePlayerID_Neutral) && !creature->HasOwner(playerId))
            return false;

        if (!creature->CanPickUp())
            return false;

        // todo: add checks

        return true;
    }

    // todo: implement
    return false;
}

bool InteractionService::PickUpEntity(EntityHandle entityHandle, ePlayerID playerId)
{
    if (!TestPickUpEntity(entityHandle, playerId))
        return false;

    // handle pickup creature
    if (entityHandle.IsCreature())
    {
        Creature* creature = gCreatureManager.GetCreaturePtr(entityHandle);
        cxx_assert(creature);

        if (!creature->PickUp())
        {
            cxx_assert(false);
            return false;
        }
        gGameSession.GetPlayer(playerId).AddHeldEntity(entityHandle);
        return true;
    }

    // todo: implement
    cxx_assert(false);
    return false;
}

bool InteractionService::TestDropEntityOn(EntityHandle entityHandle, ePlayerID playerId, const glm::vec2& position) const
{
    // handle drop creature
    if (entityHandle.IsCreature())
    {
        Creature* creature = gCreatureManager.GetCreaturePtr(entityHandle);
        if (creature == nullptr)
            return false;

        if (!gGameSession.GetPlayer(playerId).HasInHand(entityHandle))
            return false;

        // check map tile
        MapTile* mapTile = gGameMap.GetTileAtPosition(position);
        if (mapTile == nullptr)
            return false;

        TerrainDefinition* tileTerrainDef = mapTile->GetTerrain();
        if (tileTerrainDef->mIsSolid)
            return false;

        if (!gDebug.mAllowDropHeldEntityOnAnyLand)
        {
            if ((tileTerrainDef->mIsLava || tileTerrainDef->mIsWater))
                return false;

            if (tileTerrainDef->mIsOwnable && !mapTile->HasOwner(playerId))
                return false;

            if (!tileTerrainDef->mIsOwnable)
            {
                CreatureDefinition* creatureDefs = creature->GetDefinition();
                if (!creatureDefs->mIsWorker)
                    return false;
            }
        }
        // todo: add checks

        return true;
    }

    // todo: implement
    return false;
}

bool InteractionService::DropEntityOn(EntityHandle entityHandle, ePlayerID playerId, const glm::vec2& position)
{
    if (!TestDropEntityOn(entityHandle, playerId, position))
        return false;

    // handle drop creature
    if (entityHandle.IsCreature())
    {
        Creature* creature = gCreatureManager.GetCreaturePtr(entityHandle);
        cxx_assert(creature);

        if (!creature->DropOn(position))
        {
            cxx_assert(false);
            return false;
        }
        gGameSession.GetPlayer(playerId).RemoveHeldEntity(entityHandle);
        return true;
    }

    // todo: implement
    cxx_assert(false);
    return false;
}
