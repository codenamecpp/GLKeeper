#include "stdafx.h"
#include "CreatureController.h"
#include "CreatureAnimConst.h"
#include "SimplePool.h"

CreatureController::~CreatureController()
{

}

void CreatureController::ConfigureInstance(Creature* creatureInstance)
{
    cxx_assert((mCreature == nullptr) && creatureInstance);
    mCreature = creatureInstance;
}

void CreatureController::SpawnInstance()
{
    ConfigureCreatureAnimationStates();

    GetCreature().GetAnimator().Start();
}

void CreatureController::DespawnInstance()
{

}

void CreatureController::UpdateLogic(float stepDeltaTime)
{
    // update current activity
    if (CreatureActivity* currentActivity = GetCreature().GetCurrentActivity())
    {
        HandleActivity(*currentActivity);
    }

    // check current activity finish
    if (CreatureActivity* currentActivity = GetCreature().GetCurrentActivity())
    {
        if (CreatureActivityUtils::IsFinished(*currentActivity))
        {
            GetCreature().ClearCurrentActivity();
        }
    }

    // try start next activity
    if (!GetCreature().HasActivity())
    {
        GetCreature().SwitchToRequestActivity();
    }

    // look for new activities
    if (!GetCreature().HasActivity())
    {
        SelectNextActivity();
    }
}

void CreatureController::ConfigureCreatureAnimationStates()
{
    Animator& animator = GetCreature().GetAnimator();

    CreatureDefinition* definition = GetCreature().GetDefinition();

    auto GetAnimResource = [definition](CreatureAnimationID animID) -> const ArtResourceDefinition&
        {
            const ArtResourceDefinition& animResource = definition->mAnimationResources[animID];
            if (animResource.IsDefined()) return animResource;
            // fallback to pose frame
            return definition->mAnimationResources[CreatureAnimation_Pose_Frame];
        };

    // configure common states
    animator.DefineState(CreatureAnimConst::StatePose, GetAnimResource(CreatureAnimation_Pose_Frame), eAnimationLoopMode_None);
    animator.DefineState(CreatureAnimConst::StateWalk, GetAnimResource(CreatureAnimation_Walk), eAnimationLoopMode_Repeat);
    animator.DefineState(CreatureAnimConst::StateIdleSelector, GetAnimResource(CreatureAnimation_Pose_Frame), eAnimationLoopMode_None);
    animator.DefineState(CreatureAnimConst::StateIdle1, GetAnimResource(CreatureAnimation_Idle1), eAnimationLoopMode_None);
    animator.DefineState(CreatureAnimConst::StateIdle2, GetAnimResource(CreatureAnimation_Idle2), eAnimationLoopMode_None);

    // configure start transitions
    animator.DefineTransitionFromStart(CreatureAnimConst::StatePose, {});

    // configure any state transitions in order: from highest down to lowest priority
    {
        // any -> walk
        {
            //animator->DefineTransitionFromAnyState(CreatureAnimConst::StateWalk, 
            //    {
            //        Animator::ConditionForBool(CreatureAnimConst::ParamIsWalk, true)
            //    },
            //    Animator::eTransitionMode_WaitAnimation,
            //    Animator::eTransitionInterruptionSource_CurrentStateThenNextState);
        }

    }

    // configure specific state transitions

    {
        // walk ->
        {
            animator.DefineTransition(CreatureAnimConst::StateWalk, CreatureAnimConst::StatePose, {});
            animator.DefineTransition(CreatureAnimConst::StateWalk, CreatureAnimConst::StatePose,
                {
                    Animator::ConditionForBool(CreatureAnimConst::ParamIsWalk, false)
                },
                Animator::eTransitionMode_WaitAnimation,
                Animator::eTransitionInterruptionSource_None);
        }

        // idle selector ->
        {
            animator.DefineTransition(CreatureAnimConst::StateIdleSelector, CreatureAnimConst::StateIdle1, 60.0f);
            animator.DefineTransition(CreatureAnimConst::StateIdleSelector, CreatureAnimConst::StateIdle2, 40.0f);
        }

        // idle ->
        {
            animator.DefineTransition(CreatureAnimConst::StateIdle1, CreatureAnimConst::StatePose, {});
            animator.DefineTransition(CreatureAnimConst::StateIdle2, CreatureAnimConst::StatePose, {});
        }
    }

}

bool CreatureController::SelectNextActivity()
{
    GetCreature().RequestActivity<CreatureActivity_Idle>();
    return true;
}

void CreatureController::HandleNotification(const EntityNotification& notification)
{
    if (CreatureActivity* currentActivity = GetCreature().GetCurrentActivity())
    {
        HandleActivityNotification(*currentActivity, notification);
    }
}

void CreatureController::OnRecycle()
{
    DespawnInstance();
    mCreature = nullptr;
}

void CreatureController::UpdateFrame(float deltaTime)
{
    // 
}

bool CreatureController::HandleActivityNotification(CreatureActivity_None& activity, const EntityNotification& notification)
{
    return false;
}

bool CreatureController::HandleActivityNotification(CreatureActivity_GoToBed& activity, const EntityNotification& notification)
{
    return false;
}

bool CreatureController::HandleActivityNotification(CreatureActivity_GoToFood& activity, const EntityNotification& notification)
{
    return false;
}

bool CreatureController::HandleActivityNotification(CreatureActivity_Explore& activity, const EntityNotification& notification)
{
    return false;
}

bool CreatureController::HandleActivityNotification(CreatureActivity_Sleep& activity, const EntityNotification& notification)
{
    return false;
}

bool CreatureController::HandleActivityNotification(CreatureActivity_Eat& activity, const EntityNotification& notification)
{
    return false;
}

bool CreatureController::HandleActivityNotification(CreatureActivity& activity, const EntityNotification& notification)
{
    bool canHandle = false;
    std::visit([this, &notification, &canHandle](auto& a) { canHandle = this->HandleActivityNotification(a, notification); }, activity);
    return canHandle;
}

bool CreatureController::HandleActivity(CreatureActivity& activity)
{
    bool canHandle = false;
    std::visit([this, &canHandle](auto& a) { canHandle = this->HandleActivity(a); }, activity);
    return canHandle;
}

bool CreatureController::HandleActivity(CreatureActivity_None& activity)
{
    activity.SetResult((activity.GetStatus() == eCreatureActivityStatus_Cancelling) ?
        eCreatureActivityResult_Cancelled :
        eCreatureActivityResult_Success);
    return true;
}

bool CreatureController::HandleActivity(CreatureActivity_GoToBed& activity)
{
    return false;
}

bool CreatureController::HandleActivity(CreatureActivity_GoToFood& activity)
{
    return false;
}

bool CreatureController::HandleActivity(CreatureActivity_Explore& activity)
{
    return false;
}

bool CreatureController::HandleActivity(CreatureActivity_Sleep& activity)
{
    return false;
}

bool CreatureController::HandleActivity(CreatureActivity_Eat& activity)
{
    return false;
}
