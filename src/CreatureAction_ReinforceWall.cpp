#include "stdafx.h"
#include "CreatureAction_ReinforceWall.h"
#include "GameMap.h"
#include "CreatureManager.h"
#include "CreatureAnimConst.h"
#include "GameWorld.h"

CreatureAction_ReinforceWall::CreatureAction_ReinforceWall()
    : CreatureAction(eCreatureAction_ReinforceWall)
{

}

void CreatureAction_ReinforceWall::Configure(Creature* creature, const glm::vec2& workPoint, const MapPoint2D& targetTile)
{
    CreatureAction::Configure(creature);
    mWallTile = gGameMap.GetMapTileOrNull(targetTile);
    cxx_assert(mWallTile);
    mWorkPoint = workPoint;
}

void CreatureAction_ReinforceWall::OnRecycle()
{
    CreatureAction::OnRecycle();
    mWallTile = {};
    mWorkPoint = {};
    mReinforceTimer = {};
}

void CreatureAction_ReinforceWall::HandleEnterAction()
{
    if (mWallTile == nullptr)
    {
        SetActionResult(CreatureAction::eResult_Failed);
        return;
    }

    // start walk to destination
    StartSubAction(gCreatureManager.CreateWalkToPointAction(GetCreaturePtr(), mWorkPoint));
}

void CreatureAction_ReinforceWall::HandleLeaveAction()
{
    StopReinforcingWall();
}

void CreatureAction_ReinforceWall::HandleUpdateLogic(float stepDeltaTime)
{
    bool isCancelled = IsCancellationRequested();
    if (isCancelled || !ProcessReinforcingWall(stepDeltaTime))
    {
        StopReinforcingWall();
    }

    Animator& animator = GetCreature().GetAnimator();
    if (animator.IsCurrentState(CreatureAnimConst::StateReinforceWall) && !animator.IsCurrentStateStopped())
    {
        // continue with reinforcement
        return;
    }

    SetActionResult(isCancelled ? CreatureAction::eResult_Cancelled : CreatureAction::eResult_Success);
}

void CreatureAction_ReinforceWall::HandleResumeAction(eCreatureAction subActionId, eResult subActionResult)
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
        if (!MapUtils::AreTilesAdjacent(currentTile, mWallTile->mLocation))
        {
            cxx_assert(false);
            SetActionResult(CreatureAction::eResult_Failed);
            return;
        }

        // start orient to tile
        StartSubAction(gCreatureManager.CreateFaceTileAction(GetCreaturePtr(), mWallTile->mLocation));
        return;
    }

    if (subActionId == eCreatureAction_FaceTarget)
    {
        StartReinforcingWall();
        return;
    }

    cxx_assert(false);
}

void CreatureAction_ReinforceWall::StartReinforcingWall()
{
    mReinforceTimer.Start(1.0f);

    Animator& animator = GetCreature().GetAnimator();
    animator.ChangeState(CreatureAnimConst::StateReinforceWall);
    animator.SetParamValue(CreatureAnimConst::ParamIsReinforceWall, true);
}

void CreatureAction_ReinforceWall::StopReinforcingWall()
{
    Animator& animator = GetCreature().GetAnimator();
    animator.SetParamValue(CreatureAnimConst::ParamIsReinforceWall, false);
}

bool CreatureAction_ReinforceWall::ProcessReinforcingWall(float stepDeltaTime)
{
    if (!mReinforceTimer.IsOngoing())
        return false;

    cxx_assert(mWallTile);

    if (mReinforceTimer.TickAndCheckExpire(stepDeltaTime))
    {
        bool wasCompleted = false;
        if (!gGameWorld.ReinforceWall(mWallTile, GetCreature().GetOwnerId(), wasCompleted))
            return false;

        if (wasCompleted)
            return false;

        // continue with operation
        mReinforceTimer.Start();
    }

    return true;
}
