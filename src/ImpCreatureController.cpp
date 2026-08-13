#include "stdafx.h"
#include "ImpCreatureController.h"
#include "CreatureAnimConst.h"

void ImpCreatureController::OnRecycle()
{
    CreatureController::OnRecycle();

}

void ImpCreatureController::SpawnInstance()
{
    CreatureController::SpawnInstance();

    // make imps orient faster
    // todo: move to settings
    GetCreature().GetLocomotion().SetOrientSpeed(glm::radians(560.0f));
}

void ImpCreatureController::ConfigureCreatureAnimationStates()
{
    CreatureController::ConfigureCreatureAnimationStates();

    CreatureDefinition* definition = GetCreature().GetDefinition();
    auto GetAnimResource = [definition](CreatureAnimationID animID)
    {
        return definition->GetAnimResourceOrPoseFrame(animID);
    };

    // configure common states
    Animator& animator = GetCreature().GetAnimator();
    animator.DefineState(CreatureAnimConst::StateDigging, GetAnimResource(CreatureAnimation_Imp_Dig), eAnimationLoopMode_Repeat);
    animator.DefineState(CreatureAnimConst::StateReinforceWall, GetAnimResource(CreatureAnimation_Imp_ClaimLand), eAnimationLoopMode_Repeat);
    animator.DefineState(CreatureAnimConst::StateClaimFloor, GetAnimResource(CreatureAnimation_Imp_ClaimFloor), eAnimationLoopMode_Repeat);

    // configure specific state transitions

    // digging ->
    {
        animator.DefineTransition(CreatureAnimConst::StateDigging, CreatureAnimConst::StatePose, {});
        animator.DefineTransition(CreatureAnimConst::StateDigging, CreatureAnimConst::StatePose,
            {
                Animator::ConditionForBool(CreatureAnimConst::ParamIsDigging, false)
            },
            Animator::eTransitionMode_WaitAnimation,
            Animator::eTransitionInterruptionSource_None);
    }
    // reinforcing wall ->
    {
        animator.DefineTransition(CreatureAnimConst::StateReinforceWall, CreatureAnimConst::StatePose, {});
        animator.DefineTransition(CreatureAnimConst::StateReinforceWall, CreatureAnimConst::StatePose,
            {
                Animator::ConditionForBool(CreatureAnimConst::ParamIsReinforceWall, false)
            },
            Animator::eTransitionMode_Immediate,
            Animator::eTransitionInterruptionSource_None);
    }
    // claiming floor tile ->
    {
        animator.DefineTransition(CreatureAnimConst::StateClaimFloor, CreatureAnimConst::StatePose, {});
        animator.DefineTransition(CreatureAnimConst::StateClaimFloor, CreatureAnimConst::StatePose,
            {
                Animator::ConditionForBool(CreatureAnimConst::ParamIsClaimFloor, false)
            },
            Animator::eTransitionMode_Immediate,
            Animator::eTransitionInterruptionSource_None);
    }
}
