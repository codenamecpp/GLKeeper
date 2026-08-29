#include "stdafx.h"
#include "CreatureAction_WalkToPoint.h"
#include "NavigationService.h"
#include "Creature.h"
#include "CreatureAnimConst.h"

CreatureAction_WalkToPoint::CreatureAction_WalkToPoint()
    : CreatureAction(eCreatureAction_WalkToPoint)
{
}

void CreatureAction_WalkToPoint::Configure(Creature* creature, const glm::vec2& destination)
{
    CreatureAction::Configure(creature);

    mDestination = destination;
}

void CreatureAction_WalkToPoint::OnRecycle()
{
    CreatureAction::OnRecycle();

    mPathFindRequest.reset();
    mDestination = {};
}

void CreatureAction_WalkToPoint::HandleEnterAction()
{
    // request path to destination
    mPathFindRequest = gNavigationService.RequestPath(GetCreature().GetPosition2d(), mDestination, GetCreature().GetPassabilityType());
    cxx_assert(mPathFindRequest);
    if (!mPathFindRequest)
    {
        SetActionResult(CreatureAction::eResult_Failed);
    }
}

void CreatureAction_WalkToPoint::HandleLeaveAction()
{
    mPathFindRequest.reset();

    Animator& animator = GetCreature().GetAnimator();
    animator.SetParamValue(CreatureAnimConst::ParamIsWalk, false);

    Locomotion& locomotion = GetCreature().GetLocomotion();
    locomotion.ClearGoals();
}

void CreatureAction_WalkToPoint::HandleUpdateLogic(float stepDeltaTime)
{
    // processs cancellation request
    if (IsCancellationRequested())
    {
        mPathFindRequest.reset();
        SetActionResult(CreatureAction::eResult_Cancelled);
        return;
    }

    Locomotion& locomotion = GetCreature().GetLocomotion();
    Animator& animator = GetCreature().GetAnimator();

    // path searching is in progress?
    if (mPathFindRequest)
    {
        // wait completion
        if (mPathFindRequest->IsSearching())
            return;

        // configure locomotion goals
        locomotion.ClearGoals();
        for (int ipoint = 1, Count = mPathFindRequest->GetPathWaypointsCount(); ipoint < Count; ++ipoint)
        {
            const glm::vec2& point = mPathFindRequest->GetResultPathWaypoints()[ipoint];
            // most creatures seem to face the first waypoint before moving
            // the imp is one exception to this
            if ((ipoint == 1) && !GetCreature().IsCreature(CreatureTypeId_Imp))
            {
                locomotion.OrientToPoint(point, GetCreature().GetTransform());
            }
            locomotion.ArriveTo(point, true);
        }
        mPathFindRequest.reset();

        if (!locomotion.HasGoals())
        {
            SetActionResult(CreatureAction::eResult_Success);
            return;
        }

        const CreatureDefinition* creatureDefs = GetCreature().GetDefinition(); // todo: get move speed depending on current state
        locomotion.SetArriveSpeed(creatureDefs->mSpeed);

        // force pose frame while orienting
        animator.ChangeState(CreatureAnimConst::StatePose);
        return;
    }

    // continue with goals
    if (locomotion.HasGoals())
        return;

    // wait animation ends
    animator.SetParamValue(CreatureAnimConst::ParamIsWalk, false);
    if (animator.IsCurrentState(CreatureAnimConst::StateWalk) && !animator.IsCurrentStateStopped())
        return;

    // done
    SetActionResult(CreatureAction::eResult_Success);
}

void CreatureAction_WalkToPoint::HandleMessage(EntityMsg& msgData)
{
    if (msgData.Is(EntityMsg::eID_LocoApplyVelocities))
    {
        CreatureDefinition* creatureDefs = GetCreature().GetDefinition(); // todo: get move speed depending on current state

        Animator& animator = GetCreature().GetAnimator();
        // start walk?
        if (glm::length2(msgData.mLocoVelocities.mLinear) > 0.0f)
        {
            if (!animator.IsCurrentState(CreatureAnimConst::StateWalk))
            {
                animator.SetAnimSpeedFactor(CreatureAnimConst::StateWalk, creatureDefs->mWalkAnimScale * creatureDefs->mSpeed);
                animator.SetParamValue(CreatureAnimConst::ParamIsWalk, true);
                animator.ChangeState(CreatureAnimConst::StateWalk);
            }
        }
        else // stop walk
        {
            if (animator.IsCurrentState(CreatureAnimConst::StateWalk))
            {
                animator.SetParamValue(CreatureAnimConst::ParamIsWalk, false);
            }
        }
        msgData.SetConsumed();
        return;
    }

    if (msgData.Is(EntityMsg::eID_LocoClearVelocities))
    {
        Animator& animator = GetCreature().GetAnimator();
        // interrupt walk animation
        if (animator.IsCurrentState(CreatureAnimConst::StateWalk))
        {
            animator.SetParamValue(CreatureAnimConst::ParamIsWalk, false);
        }
        msgData.SetConsumed();
        return;
    }
}
