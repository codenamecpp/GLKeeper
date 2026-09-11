#include "stdafx.h"
#include "InteractionService.h"
#include "CreatureManager.h"
#include "GameSession.h"
#include "MapUtils.h"
#include "GameMap.h"
#include "GameWorld.h"
#include "EconomyService.h"
#include "Room.h"

//////////////////////////////////////////////////////////////////////////

InteractionService gInteractionService;

//////////////////////////////////////////////////////////////////////////

void InteractionService::EnterWorld()
{

}

void InteractionService::ClearWorld()
{

}

bool InteractionService::CanPickUpEntity(EntityHandle entityHandle, ePlayerID playerId) const
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

bool InteractionService::TryPickUpEntity(EntityHandle entityHandle, ePlayerID playerId)
{
    if (!CanPickUpEntity(entityHandle, playerId))
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

bool InteractionService::CanDropEntityOn(EntityHandle entityHandle, ePlayerID playerId, const glm::vec2& position) const
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

bool InteractionService::TryDropEntityOn(EntityHandle entityHandle, ePlayerID playerId, const glm::vec2& position)
{
    if (!CanDropEntityOn(entityHandle, playerId, position))
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

bool InteractionService::CanBuildRoom(ePlayerID playerId, RoomDefinition* roomDefinition, const Rect2D& mapArea, long& outBuildingCost, 
    cxx::any_vector<MapTile*> outRoomTiles) const
{
    outRoomTiles.clear();
    outBuildingCost = 0;

    cxx_assert(roomDefinition);
    const Player& player = gGameSession.GetPlayer(playerId);
    if (!player.CanBuildRoomOfType(roomDefinition))
        return false;

    bool isSuccess = gGameWorld.CanConstructRooms(playerId, roomDefinition, mapArea, outRoomTiles);

    const int roomTilesCount = outRoomTiles.size();

    outBuildingCost = gEconomyService.CalculateBuildingCost(playerId, roomDefinition, roomTilesCount);
    if (isSuccess)
    {
        isSuccess = gEconomyService.HasEnoughResources(playerId, eGameResource_Gold, outBuildingCost);
    }
    return isSuccess;
}

bool InteractionService::TryBuildRoom(ePlayerID playerId, RoomDefinition* roomDefinition, const Rect2D& mapArea, long& outBuildingCost, 
    cxx::any_vector<MapTile*> outRoomTiles)
{
    if (!CanBuildRoom(playerId, roomDefinition, mapArea, outBuildingCost, outRoomTiles))
        return false;

    outRoomTiles.clear();
    if (!gGameWorld.ConstructRooms(playerId, roomDefinition, mapArea, outRoomTiles))
    {
        cxx_assert(false);
        return false;
    }
    gEconomyService.TakeResource(playerId, eGameResource_Gold, outBuildingCost);
    return true;
}

bool InteractionService::CanSellRoom(ePlayerID playerId, const Rect2D& mapArea, long& outMoney, cxx::any_vector<MapTile*> outRoomTiles) const
{
    outRoomTiles.clear();
    outMoney = 0;

    bool isSuccess = gGameWorld.CanDemolishRooms(playerId, mapArea, outRoomTiles);

    for (MapTile* mapTile: outRoomTiles)
    {
        cxx_assert(mapTile->mRoomInstance);
        if (mapTile->mRoomInstance == nullptr)
            continue;

        RoomDefinition* roomDefinition = mapTile->mRoomInstance->GetDefinition();
        cxx_assert(roomDefinition);

        const long sellValue = gEconomyService.CalculateSellingValue(playerId, roomDefinition, 1);
        cxx_assert(sellValue > 0);
        outMoney += sellValue;
    }
    return isSuccess;
}

bool InteractionService::TrySellRoom(ePlayerID playerId, const Rect2D& mapArea, long& outMoney, cxx::any_vector<MapTile*> outRoomTiles)
{
    if (!CanSellRoom(playerId, mapArea, outMoney, outRoomTiles))
        return false;

    outRoomTiles.clear();
    if (!gGameWorld.DemolishRooms(playerId, mapArea, outRoomTiles))
    {
        cxx_assert(false);
        return false;
    }
    long resourceStored = gEconomyService.GiveResource(playerId, eGameResource_Gold, outMoney);
    if (resourceStored < outMoney)
    {
        // handle excess amount of resource that exceeds the storage capacity
        long excessMoney = outMoney - resourceStored;
        // todo
    }
    return true;
}
