#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "PlayerDefs.h"

//////////////////////////////////////////////////////////////////////////

class EconomyService final: public cxx::noncopyable
{
public:
    void EnterWorld();
    void ClearWorld();

    void UpdateFrame(float deltaTime);
    void UpdateLogic(float stepDeltaTime);

    bool HasEnoughResources(ePlayerID playerId, eGameResource resourceType, long neededAmount) const;

    // player resource management
    long GiveResource(ePlayerID playerId, eGameResource resourceType, long resourceAmount);
    long TakeResource(ePlayerID playerId, eGameResource resourceType, long resourceAmount);

    // costs calculation
    long CalculateBuildingCost(ePlayerID playerId, RoomDefinition* roomDefinition, int tileCount) const;
    long CalculateSellingValue(ePlayerID playerId, RoomDefinition* roomDefinition, int tileCount) const;

public:
    // notifications
    void StoredMoneyAmountChanged(ePlayerID playerId, EntityHandle roomHandle, long amountDelta);

private:
    long GiveResource(Player& player, eGameResource resourceType, long resourceAmount);
    long TakeResource(Player& player, eGameResource resourceType, long resourceAmount);

    void IssueStartingResources();

private:
};

//////////////////////////////////////////////////////////////////////////

extern EconomyService gEconomyService;

//////////////////////////////////////////////////////////////////////////