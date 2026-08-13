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

    CreatureDefinition* definition = GetCreature().GetDefinition();

    // add common components

    if ((definition->mMaxGoldHeld > 0) || (definition->mInitialGoldHeld))
    {
        MoneyComponent* moneyComponent = GetCreature().AddComponent<MoneyComponent>();
        cxx_assert(moneyComponent);
        moneyComponent->mAmount = definition->mInitialGoldHeld;
        moneyComponent->mCapacity = definition->mMaxGoldHeld;
    }
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

}

void CreatureController::ConfigureCreatureAnimationStates()
{
    Animator& animator = GetCreature().GetAnimator();

    CreatureDefinition* definition = GetCreature().GetDefinition();

    auto GetAnimResource = [definition](CreatureAnimationID animID)
        {
            return definition->GetAnimResourceOrPoseFrame(animID);
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

void CreatureController::OnRecycle()
{
    DespawnInstance();
    mCreature = nullptr;
}

void CreatureController::UpdateFrame(float deltaTime)
{
    // 
}

void CreatureController::HandleMessage(const EntityMsg& msgData)
{
}