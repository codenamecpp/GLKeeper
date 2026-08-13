#include "stdafx.h"
#include "CreatureAction_IdleStanding.h"
#include "Creature.h"
#include "CreatureAnimConst.h"

CreatureAction_IdleStanding::CreatureAction_IdleStanding()
    : CreatureAction(eCreatureAction_IdleStanding)
{
}

void CreatureAction_IdleStanding::HandleEnterAction()
{
    Animator& animator = GetCreature().GetAnimator();
    // start random idle animation
    animator.ChangeState(CreatureAnimConst::StateIdleSelector);
}

void CreatureAction_IdleStanding::HandleLeaveAction()
{
    Animator& animator = GetCreature().GetAnimator();
    // reset to pose
    animator.ChangeState(CreatureAnimConst::StatePose);
}

void CreatureAction_IdleStanding::HandleUpdateLogic(float stepDeltaTime)
{
    Animator& animator = GetCreature().GetAnimator();

    if (!CreatureAnimConst::IsIdleState(animator.GetCurrentState()) || animator.IsCurrentStateStopped())
    {
        SetActionResult(CreatureAction::eResult_Success);
        return;
    }

    if (IsCancellationRequested())
    {
        SetActionResult(CreatureAction::eResult_Cancelled);
    }
}