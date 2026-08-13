#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureAction.h"
#include "SimpleTimer.h"

//////////////////////////////////////////////////////////////////////////

class CreatureAction_ReinforceWall: public CreatureAction
{
public:
    CreatureAction_ReinforceWall();
    void Configure(Creature* creature, const glm::vec2& workPoint, const MapPoint2D& targetTile);

    // override CreatureAction
    void OnRecycle() override;
    void HandleEnterAction() override;
    void HandleLeaveAction() override;
    void HandleUpdateLogic(float stepDeltaTime) override;
    void HandleResumeAction(eCreatureAction subActionId, eResult subActionResult) override;

private:
    void StartReinforcingWall();
    void StopReinforcingWall();
    bool ProcessReinforcingWall(float stepDeltaTime);

private:
    glm::vec2 mWorkPoint {};
    MapTile* mWallTile {};
    SimpleTimer mReinforceTimer {};
};

//////////////////////////////////////////////////////////////////////////