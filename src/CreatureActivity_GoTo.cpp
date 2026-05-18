#include "stdafx.h"
#include "CreatureController.h"
#include "CreatureAnimConst.h"

bool CreatureController::HandleActivity(CreatureActivity_GoTo& activity)
{
    cxx_assert(activity.GetStatus() != eCreatureActivityStatus_Finished);

    CreatureDefinition* creatureDefs = GetCreature().GetDefinition(); // todo: get move speed depending on current state

    Animator& animator = GetCreature().GetAnimator();
    Locomotion& locomotion = GetCreature().GetLocomotion();

    //////////////////////////////////////////////////////////////////////////
    // starting
    //////////////////////////////////////////////////////////////////////////

    if (activity.GetStatus() == eCreatureActivityStatus_Init)
    {
        locomotion.ClearGoals();
        locomotion.SetArriveSpeed(creatureDefs->mSpeed);
        // setup guals
        locomotion.OrientToPoint(activity.GetDestination(), GetCreature().GetTransform());
        locomotion.ArriveTo(activity.GetDestination(), false);
        // random turn at the end
        if (Random::GenerateChance(50))
        {
            locomotion.OrientTo(Random::GenerateAngle());
        }
        // force pose frame while orienting
        animator.ChangeState(CreatureAnimConst::StatePose);

        activity.SetStatus(eCreatureActivityStatus_Running);
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    // continue
    //////////////////////////////////////////////////////////////////////////

    if (activity.GetStatus() == eCreatureActivityStatus_Running)
    {
        if (locomotion.HasGoals())
            return true;

        // wait animation end
        animator.SetParamValue(CreatureAnimConst::ParamIsWalk, false);
        if (animator.IsCurrentState(CreatureAnimConst::StateWalk) && !animator.IsCurrentStateStopped())
            return true;

        activity.SetResult(eCreatureActivityResult_Success);
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    // stopping
    //////////////////////////////////////////////////////////////////////////

    if (activity.GetStatus() == eCreatureActivityStatus_Cancelling)
    {
        locomotion.ClearGoals();

        // cancel animation
        animator.SetParamValue(CreatureAnimConst::ParamIsWalk, false);

        activity.SetResult(eCreatureActivityResult_Cancelled);
        return true;
    }

    return false;
}

bool CreatureController::HandleActivityNotification(CreatureActivity_GoTo& activity, const EntityNotification& notification)
{
    // handle locomotion


    if (notification.Is(EntityNotification::eID_LocoApplyVelocities))
    {
        CreatureDefinition* creatureDefs = GetCreature().GetDefinition(); // todo: get move speed depending on current state

        Animator& animator = GetCreature().GetAnimator();
        // start walk?
        if (glm::length2(notification.mLocoVelocities.mLinear) > 0.0f)
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
        return true;
    }

    if (notification.Is(EntityNotification::eID_LocoClearVelocities))
    {
        Animator& animator = GetCreature().GetAnimator();
        // interrupt walk animation
        if (animator.IsCurrentState(CreatureAnimConst::StateWalk))
        {
            animator.SetParamValue(CreatureAnimConst::ParamIsWalk, false);
        }
        return true;
    }

    return false;
}