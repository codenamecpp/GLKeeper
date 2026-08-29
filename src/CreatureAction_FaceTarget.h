#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureAction.h"

//////////////////////////////////////////////////////////////////////////

class CreatureAction_FaceTarget: public CreatureAction
{
public:
    CreatureAction_FaceTarget();

    void Configure(Creature* creature, const Point2D& mapTile);

    // override CreatureAction
    void OnRecycle() override;
    void HandleEnterAction() override;
    void HandleLeaveAction() override;
    void HandleUpdateLogic(float stepDeltaTime) override;
    void HandleResumeAction(eCreatureAction subActionId, eResult subActionResult) override;

private:
    std::optional<Point2D> mTargetMapTile;
};

//////////////////////////////////////////////////////////////////////////