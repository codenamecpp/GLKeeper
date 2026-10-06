#include "stdafx.h"
#include "CreatureState_InHand.h"
#include "Creature.h"

CreatureState_InHand::CreatureState_InHand()
    : CreatureState(eCreatureState_InHand)
{
}

void CreatureState_InHand::HandleEnterState(eCreatureState prevState)
{
    GetCreature().UnassignCurrentTask();
    if (Locomotion* locomotion = GetCreature().GetLocomotion())
    {
        locomotion->ClearGoals();
    }
    GetCreature().SetHighlighted(false);
    GetCreature().EnablePhysics(false);
    GetCreature().EnableMesh(false);
}

void CreatureState_InHand::HandleLeaveState(eCreatureState nextState)
{
    GetCreature().EnablePhysics(true);
    GetCreature().EnableMesh(true);
}

void CreatureState_InHand::HandleUpdateLogic(float stepDeltaTime)
{

}
