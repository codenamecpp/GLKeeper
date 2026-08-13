#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureAction.h"

//////////////////////////////////////////////////////////////////////////

class CreatureAction_CarryGoldToTreasury: public CreatureAction
{
public:
    CreatureAction_CarryGoldToTreasury();
    void Configure(Creature* creature, const MapPoint2D& targetTile);

    // override CreatureAction
    void OnRecycle() override;
    void HandleEnterAction() override;
    void HandleLeaveAction() override;
    void HandleUpdateLogic(float stepDeltaTime) override;
    void HandleResumeAction(eCreatureAction subActionId, eResult subActionResult) override;

private:
    MapTile* mTargetMapTile {};
};

//////////////////////////////////////////////////////////////////////////