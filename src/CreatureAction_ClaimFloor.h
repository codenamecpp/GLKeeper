#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureAction.h"
#include "SimpleTimer.h"

//////////////////////////////////////////////////////////////////////////

class CreatureAction_ClaimFloor: public CreatureAction
{
public:
    CreatureAction_ClaimFloor();
    void Configure(Creature* creature, const Point2D& targetTile);

    // override CreatureAction
    void OnRecycle() override;
    void HandleEnterAction() override;
    void HandleLeaveAction() override;
    void HandleUpdateLogic(float stepDeltaTime) override;
    void HandleResumeAction(eCreatureAction subActionId, eResult subActionResult) override;

private:
    void StartClaimFloor();
    void StopClaimFloor();
    bool ProcessClaimFloor(float stepDeltaTime);

private:
    MapTile* mFloorTile {};
    SimpleTimer mClaimTimer {};
};

//////////////////////////////////////////////////////////////////////////