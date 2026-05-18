#include "stdafx.h"
#include "EconomyService.h"
#include "GameSession.h"
#include "GameEventBus.h"
#include "RoomManager.h"

void EconomyService::EnterWorld()
{
    IssueStartingResources();
}

void EconomyService::ClearWorld()
{

}

void EconomyService::UpdateFrame(float deltaTime)
{

}

void EconomyService::UpdateLogic(float stepDeltaTime)
{

}

long EconomyService::GiveResource(ePlayerID playerId, eGameResource resourceType, long resourceAmount)
{
    return GiveResource(GetGameSession().GetPlayer(playerId), resourceType, resourceAmount);
}

long EconomyService::GiveResource(Player& player, eGameResource resourceType, long resourceAmount)
{
    if (resourceAmount < 1) return 0;

    if (player.IsNonPlayer())
    {
        cxx_assert(false);
        return 0;
    }

    if (resourceType == eGameResource_Gold)
    {
        long moneyAmountLeft = resourceAmount;
        for (const EntityHandle roomHandle: player.GetOwnedMoneyStorageRooms())
        {
            Room* roomInstance = GetRoomManager().GetRoomPtr(roomHandle);
            cxx_assert(roomInstance);
            if (roomInstance == nullptr) continue;
            if (auto* moneyStorage = roomInstance->GetCapability<MoneyStorageRoomCapability>())
            {
                long storedMoney = moneyStorage->StoreGold(moneyAmountLeft);
                moneyAmountLeft -= storedMoney;

                if (moneyAmountLeft <= 0) break;
            }
        }
        cxx_assert(moneyAmountLeft >= 0);
        return (resourceAmount - moneyAmountLeft);
    }
    
    if (resourceType == eGameResource_Mana)
    {
        cxx_assert(false);
    }

    return 0;
}

long EconomyService::TakeResource(ePlayerID playerId, eGameResource resourceType, long resourceAmount)
{
    return TakeResource(GetGameSession().GetPlayer(playerId), resourceType, resourceAmount);
}

long EconomyService::TakeResource(Player& player, eGameResource resourceType, long resourceAmount)
{
    if (resourceAmount < 1) return 0;

    if (player.IsNonPlayer())
    {
        cxx_assert(false);
        return 0;
    }

    if (resourceType == eGameResource_Gold)
    {
        long moneyAmountLeft = resourceAmount;
        for (const EntityHandle roomHandle: player.GetOwnedMoneyStorageRooms())
        {
            Room* roomInstance = GetRoomManager().GetRoomPtr(roomHandle);
            cxx_assert(roomInstance);
            if (roomInstance == nullptr) continue;
            if (auto* moneyStorage = roomInstance->GetCapability<MoneyStorageRoomCapability>())
            {
                long removedMoney = moneyStorage->DisposeGold(moneyAmountLeft);
                moneyAmountLeft -= removedMoney;

                if (moneyAmountLeft <= 0) break;
            }
        }
        cxx_assert(moneyAmountLeft >= 0);
        return (resourceAmount - moneyAmountLeft);
    }

    if (resourceType == eGameResource_Mana)
    {
        cxx_assert(false);
    }

    return 0;
}

void EconomyService::StoredGoldAmountChanged(ePlayerID playerId, EntityHandle roomHandle, long amountDelta)
{
    cxx_assert(amountDelta != 0);
    if (amountDelta != 0)
    {
        // update cache
        Player& player = GetGameSession().GetPlayer(playerId);
        player.ChangeResourceAmount(eGameResource_Gold, amountDelta);

        // issue global event
        GetGameEventBus().Send_ResourceAmountChanged(playerId, eGameResource_Gold);
    }
}

void EconomyService::IssueStartingResources()
{
    for (Player& player: GetGameSession().GetPlayers())
    {
        if (player.IsNonPlayer()) continue;

        // money
        long startingMoney = player.GetStartingResourceAmount(eGameResource_Gold);
        if (startingMoney > 0)
        {
            GiveResource(player, eGameResource_Gold, startingMoney);
        }
    }
}
