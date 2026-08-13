#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureAction.h"
#include "SimpleTimer.h"

//////////////////////////////////////////////////////////////////////////

class CreatureAction_Mining: public CreatureAction
{
public:
    CreatureAction_Mining();

    void Configure(Creature* creature, const glm::vec2& workPoint, const MapPoint2D& targetTile);

    // override CreatureAction
    void OnRecycle() override;
    void HandleEnterAction() override;
    void HandleLeaveAction() override;
    void HandleUpdateLogic(float stepDeltaTime) override;
    void HandleResumeAction(eCreatureAction subActionId, eResult subActionResult) override;

private:
    void StartTileMining();
    void StopTileMining();
    bool ProcessTileMining(float stepDeltaTime);

private:
    glm::vec2 mWorkPoint {};
    MapPoint2D mTargetTile {};
    SimpleTimer mMineTimer {};
};

//////////////////////////////////////////////////////////////////////////