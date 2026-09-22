#include "stdafx.h"
#include "QueryService.h"
#include "GameSession.h"
#include "CreatureManager.h"
#include "RoomManager.h"
#include "NavigationService.h"

//////////////////////////////////////////////////////////////////////////

QueryService gQueryService;

//////////////////////////////////////////////////////////////////////////

void QueryService::EnterWorld()
{

}

void QueryService::ClearWorld()
{

}

bool QueryService::QueryAccessibleMoneyStorageRoomsForDeposit(ePlayerID playerId, EntityHandle agentEntity, int maxRooms, 
    cxx::any_vector<EntityHandle> outEntities)
{
    outEntities.reserve(16);
    // process creature
    if (agentEntity.IsCreature())
    {
        Player& player = gGameSession.GetPlayer(playerId);
        cxx_assert(!player.IsNonPlayer());

        Creature* creature = gCreatureManager.GetCreaturePtr(agentEntity);
        if ((creature == nullptr) || !creature->ExistsOnMap())
        {
            return false;
        }

        const Point2D startTileCoord = creature->GetTilePosition();
        const ePassabilityType passabilityType = creature->GetPassabilityType();

        for (const EntityHandle& roomHandle: player.GetOwnedMoneyStorageRooms())
        {
            Room* room = gRoomManager.GetRoomPtr(roomHandle);
            if ((room == nullptr) || !room->ExistsOnMap())
            {
                continue;
            }

            // check if room is reachable
            cxx::span<MapTile*> roomTiles = room->GetFloorTiles();
            if (roomTiles.empty())
            {
                continue;
            }

            if (!gNavigationService.CheckPathExists(startTileCoord, roomTiles[0]->mLocation, passabilityType))
                continue;
           
            auto* moneyStorage = room->GetCapability<MoneyStorageRoomCapability>();
            cxx_assert(moneyStorage);
            if (moneyStorage == nullptr)
            {
                continue;
            }

            const long storageCapacity = moneyStorage->GetStoredGoldCapacity();
            bool isStorageFull = (storageCapacity > 0) && (moneyStorage->GetStoredGoldAmount() >= storageCapacity);
            if (isStorageFull)
            {
                continue;
            }
            outEntities.push_back(room->GetOwnHandle());
            
            // check limits
            if (maxRooms == static_cast<int>(outEntities.size()))
            {
                break;
            }
        } // for
    }
    else
    {
        cxx_assert(false);
    }
    bool isSuccess = !outEntities.empty();
    return isSuccess;
}


