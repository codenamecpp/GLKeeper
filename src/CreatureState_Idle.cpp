#include "stdafx.h"
#include "CreatureState_Idle.h"
#include "CreatureAction.h"
#include "Creature.h"
#include "CreatureAnimConst.h"
#include "CreatureTaskManager.h"
#include "CreatureManager.h"

CreatureState_Idle::CreatureState_Idle()
    : CreatureState(eCreatureState_Idle)
{
}

void CreatureState_Idle::HandleEnterState(eCreatureState prevState)
{
    // start idle standing
    StartStateAction(gCreatureManager.CreateIdleStandingAction(GetCreaturePtr()));
}

void CreatureState_Idle::HandleLeaveState(eCreatureState nextState)
{
}

void CreatureState_Idle::HandleUpdateLogic(float stepDeltaTime)
{
    // process idle standing
    if (CreatureAction* idleAction = GetStateAction())
    {
        if (idleAction->InProgress())
            return;

        StartStateAction(nullptr);
    }

    // try find something to do
    if (GetCreature().GetAssignedTask() || 
        GetCreature().SelectBestTask())
    {
        GetCreature().ChangeState(eCreatureState_Working);
        return;
    }

    // continue standing
    StartStateAction(gCreatureManager.CreateIdleStandingAction(GetCreaturePtr()));
}
