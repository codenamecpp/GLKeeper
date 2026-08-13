#include "stdafx.h"
#include "CreatureAction_Digging.h"
#include "CreatureManager.h"
#include "CreatureAnimConst.h"
#include "GameMap.h"
#include "GameSession.h"
#include "GameWorld.h"

CreatureAction_Digging::CreatureAction_Digging()
    : CreatureAction(eCreatureAction_Digging)
{
}

void CreatureAction_Digging::Configure(Creature* creature, const glm::vec2& workPoint, const MapPoint2D& targetTile)
{
    CreatureAction::Configure(creature);
    mWorkPoint = workPoint;
    mTargetTile = targetTile;
}

void CreatureAction_Digging::OnRecycle()
{
    CreatureAction::OnRecycle();
    mWorkPoint = {};
    mTargetTile = {};
    mDigTimer = {};
}

void CreatureAction_Digging::HandleEnterAction()
{
    // start walk to destination
    StartSubAction(gCreatureManager.CreateWalkToPointAction(GetCreaturePtr(), mWorkPoint));
}

void CreatureAction_Digging::HandleLeaveAction()
{
    StopTileDigging();
}

void CreatureAction_Digging::HandleUpdateLogic(float stepDeltaTime)
{
    Animator& animator = GetCreature().GetAnimator();

    if (IsCancellationRequested() || !ProcessTileDigging(stepDeltaTime))
    {
        StopTileDigging();
    }

    if (animator.IsCurrentState(CreatureAnimConst::StateDigging) && !animator.IsCurrentStateStopped())
    {
        // continue with digging
        return;
    }

    SetActionResult(IsCancellationRequested() ? 
        CreatureAction::eResult_Cancelled : 
        CreatureAction::eResult_Success);
}

void CreatureAction_Digging::HandleResumeAction(eCreatureAction subActionId, eResult subActionResult)
{
    if (IsCancellationRequested())
    {
        SetActionResult(CreatureAction::eResult_Cancelled);
        return;
    }

    if (subActionId == eCreatureAction_WalkToPoint)
    {
        if (subActionResult != CreatureAction::eResult_Success)
        {
            SetActionResult(subActionResult);
            return;
        }

        // check distance
        const MapPoint2D currentTile = GetCreature().GetTilePosition();
        if (!MapUtils::AreTilesAdjacent(currentTile, mTargetTile))
        {
            cxx_assert(false);
            SetActionResult(CreatureAction::eResult_Failed);
            return;
        }

        // start orient to tile
        StartSubAction(gCreatureManager.CreateFaceTileAction(GetCreaturePtr(), mTargetTile));
        return;
    }

    if (subActionId == eCreatureAction_FaceTarget)
    {
        StartTileDigging();
        return;
    }

    cxx_assert(false);
}

void CreatureAction_Digging::StartTileDigging()
{
    mDigTimer.Start(1.0f);

    Animator& animator = GetCreature().GetAnimator();
    animator.ChangeState(CreatureAnimConst::StateDigging);
    animator.SetParamValue(CreatureAnimConst::ParamIsDigging, true);
}

void CreatureAction_Digging::StopTileDigging()
{
    Animator& animator = GetCreature().GetAnimator();
    animator.SetParamValue(CreatureAnimConst::ParamIsDigging, false);
}

bool CreatureAction_Digging::ProcessTileDigging(float stepDeltaTime)
{
    if (!mDigTimer.IsOngoing())
        return false;

    MapTile* targetTile = gGameMap.GetMapTile(mTargetTile);
    cxx_assert(targetTile);
    if (targetTile == nullptr)
    {
        return false;
    }

    if (mDigTimer.TickAndCheckExpire(stepDeltaTime))
    {
        long goldMined = 0;
        if (!gGameWorld.DigTile(targetTile, GetCreature().GetOwnerId(), goldMined))
            return false;

        cxx_assert(goldMined == 0);
        mDigTimer.Start();
    }
    return true;
}
