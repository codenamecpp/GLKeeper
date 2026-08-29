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

    // player resource management
    long GiveResource(ePlayerID playerId, eGameResource resourceType, long resourceAmount);
    long TakeResource(ePlayerID playerId, eGameResource resourceType, long resourceAmount);
    long GiveResource(Player& player, eGameResource resourceType, long resourceAmount);
    long TakeResource(Player& player, eGameResource resourceType, long resourceAmount);

    // notifications
    void StoredMoneyAmountChanged(ePlayerID playerId, EntityHandle roomHandle, long amountDelta);

private:
    void IssueStartingResources();


private:
};

//////////////////////////////////////////////////////////////////////////

extern EconomyService gEconomyService;

//////////////////////////////////////////////////////////////////////////