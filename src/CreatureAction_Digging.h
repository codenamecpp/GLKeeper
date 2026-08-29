#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureAction.h"
#include "SimpleTimer.h"

//////////////////////////////////////////////////////////////////////////

class CreatureAction_Digging: public CreatureAction
{
public:
    CreatureAction_Digging();

    void Configure(Creature* creature, const glm::vec2& workPoint, const Point2D& targetTile);

    // override CreatureAction
    void OnRecycle() override;
    void HandleEnterAction() override;
    void HandleLeaveAction() override;
    void HandleUpdateLogic(float stepDeltaTime) override;
    void HandleResumeAction(eCreatureAction subActionId, eResult subActionResult) override;

private:
    void StartTileDigging();
    void StopTileDigging();
    bool ProcessTileDigging(float stepDeltaTime);

private:
    glm::vec2 mWorkPoint {};
    Point2D mTargetTile {};
    SimpleTimer mDigTimer {};
};

//////////////////////////////////////////////////////////////////////////