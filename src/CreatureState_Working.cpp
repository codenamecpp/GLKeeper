#include "stdafx.h"
#include "CreatureState_Working.h"
#include "Creature.h"
#include "CreatureTask.h"
#include "CreatureAction.h"
#include "CreatureManager.h"
#include "CreatureTaskManager.h"

CreatureState_Working::CreatureState_Working()
    : CreatureState(eCreatureState_Working)
{
}

void CreatureState_Working::HandleEnterState(eCreatureState prevState)
{
    CreatureTask* assignedTask = GetCreature().GetAssignedTask();
    if (!StartActionForTask(assignedTask))
    {
        GetCreature().UnassignCurrentTask();
    }
}

void CreatureState_Working::HandleLeaveState(eCreatureState nextState)
{
    GetCreature().UnassignCurrentTask();
}

void CreatureState_Working::HandleUpdateLogic(float stepDeltaTime)
{
    CreatureAction* creatureAction = GetStateAction();
    if (creatureAction && creatureAction->InProgress())
    {
        // handle interruption
        CreatureTask* assignedTask = GetCreature().GetAssignedTask();
        if ((assignedTask == nullptr) || assignedTask->IsExpired())
        {
            creatureAction->RequestCancellation();
        }
        return;
    }

    StartStateAction(nullptr);

    // try find new task right away
    if (GetCreature().SelectTaskForJob(GetCreature().GetLastAssignedJob()))
    {
        if (StartActionForTask(GetCreature().GetAssignedTask()))
            return;

        cxx_assert(false);
        GetCreature().UnassignCurrentTask();
    }

    GetCreature().ChangeState(eCreatureState_Idle);
}

bool CreatureState_Working::StartActionForTask(CreatureTask* creatureTask)
{
    if (creatureTask == nullptr)
        return false;

    if (creatureTask->IsExpired())
        return false;

    // todo: refactore

    switch (creatureTask->GetJobType())
    {
        case eCreatureJob_Wander:
        {
            glm::vec2 wanderPoint {};
            if (creatureTask->GetTargetPosition(wanderPoint))
            {
                StartStateAction(gCreatureManager.CreateWanderAction(GetCreaturePtr(), wanderPoint));
                return true;
            }
        }
        break;

        case eCreatureJob_Dig:
        {
            glm::vec2 workPoint {};
            MapPoint2D targetTile {};
            if (creatureTask->GetTargetPosition(workPoint) && 
                creatureTask->GetTargetTile(targetTile))
            {
                StartStateAction(gCreatureManager.CreateDiggingAction(GetCreaturePtr(), workPoint, targetTile));
                return true;
            }
        }
        break;

        case eCreatureJob_Mine:
        {
            glm::vec2 workPoint {};
            MapPoint2D targetTile {};
            if (creatureTask->GetTargetPosition(workPoint) && 
                creatureTask->GetTargetTile(targetTile))
            {
                StartStateAction(gCreatureManager.CreateMiningAction(GetCreaturePtr(), workPoint, targetTile));
                return true;
            }
        }
        break;

        case eCreatureJob_CarryGoldToTreasury:
        {
            MapPoint2D targetTile {};
            if (creatureTask->GetTargetTile(targetTile))
            {
                StartStateAction(gCreatureManager.CreateCarryGoldToTreasuryAction(GetCreaturePtr(), targetTile));
                return true;
            }
        }
        break;

        case eCreatureJob_ReinforceWall:
        {
            glm::vec2 workPoint {};
            MapPoint2D targetTile {};
            if (creatureTask->GetTargetPosition(workPoint) && 
                creatureTask->GetTargetTile(targetTile))
            {
                StartStateAction(gCreatureManager.CreateReinforceWallAction(GetCreaturePtr(), workPoint, targetTile));
                return true;
            }
        }
        break;

        case eCreatureJob_Claim:
        {
            MapPoint2D targetTile {};
            if (creatureTask->GetTargetTile(targetTile))
            {
                StartStateAction(gCreatureManager.CreateClaimFloorAction(GetCreaturePtr(), targetTile));
                return true;
            }
        }
        break;
    }

    cxx_assert(false);
    return false;
}
