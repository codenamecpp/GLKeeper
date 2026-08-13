#include "stdafx.h"
#include "CreatureAction_Wander.h"
#include "CreatureManager.h"

CreatureAction_Wander::CreatureAction_Wander()
    : CreatureAction(eCreatureAction_Wander)
{
}

void CreatureAction_Wander::Configure(Creature* creature, const glm::vec2& destination)
{
    CreatureAction::Configure(creature);
    mDestination = destination;
}

void CreatureAction_Wander::OnRecycle()
{
    CreatureAction::OnRecycle();
    mDestination = {};
}

void CreatureAction_Wander::HandleEnterAction()
{
    // start walk to destination
    StartSubAction(gCreatureManager.CreateWalkToPointAction(GetCreaturePtr(), mDestination));
}

void CreatureAction_Wander::HandleLeaveAction()
{
}

void CreatureAction_Wander::HandleUpdateLogic(float stepDeltaTime)
{
    cxx_assert(false);
    SetActionResult(CreatureAction::eResult_Failed);
}

void CreatureAction_Wander::HandleResumeAction(eCreatureAction subActionId, eResult subActionResult)
{
    cxx_assert(subActionId == eCreatureAction_WalkToPoint);
    SetActionResult(subActionResult);
}
