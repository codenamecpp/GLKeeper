#include "stdafx.h"
#include "CreatureController.h"
#include "CreatureAnimConst.h"
#include "GameWorld.h"
#include "NavigationService.h"

bool CreatureController::HandleActivity(CreatureActivity_Idle& activity)
{
    cxx_assert(activity.GetStatus() != eCreatureActivityStatus_Finished);

    Animator& animator = GetCreature().GetAnimator();

    //////////////////////////////////////////////////////////////////////////
    // starting
    //////////////////////////////////////////////////////////////////////////

    if (activity.GetStatus() == eCreatureActivityStatus_Init)
    {
        // start random idle animation
        animator.ChangeState(CreatureAnimConst::StateIdleSelector);

        activity.SetStatus(eCreatureActivityStatus_Running);
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    // continue
    //////////////////////////////////////////////////////////////////////////

    if (activity.GetStatus() == eCreatureActivityStatus_Running)
    {
        // going to somewhere?
        if (CreatureActivity* gotoActivity = activity.mGoToRandomPoint.get())
        {
            if (HandleActivity(*gotoActivity) && !CreatureActivityUtils::IsFinished(*gotoActivity))
                return true;

            activity.mGoToRandomPoint.reset();

            // stand idle
            animator.ChangeState(CreatureAnimConst::StateIdleSelector);
            return true;
        }

        // wait unit idle animation done
        if (CreatureAnimConst::IsIdleState(animator.GetCurrentState()))
        {
            if (!animator.IsCurrentStateStopped()) return true;
        }

        // with 30% chance go to somewhere
        if (Random::GenerateChance(100))
        {
            NavigationService& navigation = GetGameWorld().GetNavigationService();

            glm::vec2 destinationPoint;
            if (navigation.GetRandomWanderingPointLocation(GetCreature().GetTilePosition(), 1, 5, destinationPoint))
            {
                cxx_assert(!activity.mGoToRandomPoint);
                // launch goto activity
                activity.mGoToRandomPoint = CreatureActivityUtils::Construct<CreatureActivity_GoTo>(destinationPoint);
                cxx_assert(activity.mGoToRandomPoint);
            }

            if (CreatureActivity* gotoActivity = activity.mGoToRandomPoint.get())
            {
                if (HandleActivity(*gotoActivity)) return true;
            }
        }

        // continue standing idle
        animator.ChangeState(CreatureAnimConst::StateIdleSelector);
        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    // stopping
    //////////////////////////////////////////////////////////////////////////

    if (activity.GetStatus() == eCreatureActivityStatus_Cancelling)
    {
        // cancel goto activity
        if (CreatureActivity* gotoActivity = activity.mGoToRandomPoint.get())
        {
            CreatureActivityUtils::SetCancellationStatus(*gotoActivity);
            if (HandleActivity(*gotoActivity) && !CreatureActivityUtils::IsFinished(*gotoActivity))
                return true;

            activity.mGoToRandomPoint.reset();
        }

        activity.SetResult(eCreatureActivityResult_Cancelled);
        return true;
    }

    return false;
}

bool CreatureController::HandleActivityNotification(CreatureActivity_Idle& activity, const EntityNotification& notification)
{
    if (CreatureActivity* gotoActivity = activity.mGoToRandomPoint.get())
    {
        if (HandleActivityNotification(*gotoActivity, notification))
            return true;
    }
    
    return false;
}
