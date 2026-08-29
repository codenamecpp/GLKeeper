#include "stdafx.h"
#include "CreatureAction_ClaimFloor.h"
#include "GameMap.h"
#include "CreatureManager.h"
#include "CreatureAnimConst.h"
#include "GameWorld.h"

CreatureAction_ClaimFloor::CreatureAction_ClaimFloor()
    : CreatureAction(eCreatureAction_ClaimFloor)
{
}

void CreatureAction_ClaimFloor::Configure(Creature* creature, const Point2D& targetTile)
{
    CreatureAction::Configure(creature);
    mFloorTile = gGameMap.GetMapTileOrNull(targetTile);
    cxx_assert(mFloorTile);
}

void CreatureAction_ClaimFloor::OnRecycle()
{
    CreatureAction::OnRecycle();
    mFloorTile = nullptr;
    mClaimTimer = {};
}

void CreatureAction_ClaimFloor::HandleEnterAction()
{
    if (mFloorTile == nullptr)
    {
        SetActionResult(CreatureAction::eResult_Failed);
        return;
    }

    // start walk to destination
    glm::vec2 workPoint = MapUtils::ComputeTileCenter2d(mFloorTile->mLocation);
    StartSubAction(gCreatureManager.CreateWalkToPointAction(GetCreaturePtr(), workPoint));
}

void CreatureAction_ClaimFloor::HandleLeaveAction()
{
    StopClaimFloor();
}

void CreatureAction_ClaimFloor::HandleUpdateLogic(float stepDeltaTime)
{
    bool isCancelled = IsCancellationRequested();
    if (isCancelled || !ProcessClaimFloor(stepDeltaTime))
    {
        StopClaimFloor();
    }

    Animator& animator = GetCreature().GetAnimator();
    if (animator.IsCurrentState(CreatureAnimConst::StateClaimFloor) && !animator.IsCurrentStateStopped())
    {
        // continue with claiming
        return;
    }

    SetActionResult(isCancelled ? CreatureAction::eResult_Cancelled : CreatureAction::eResult_Success);
}

void CreatureAction_ClaimFloor::HandleResumeAction(eCreatureAction subActionId, eResult subActionResult)
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
        const Point2D currentTile = GetCreature().GetTilePosition();
        if (currentTile != mFloorTile->mLocation)
        {
            cxx_assert(false);
            SetActionResult(CreatureAction::eResult_Failed);
            return;
        }

        StartClaimFloor();
        return;
    }

    cxx_assert(false);
}

void CreatureAction_ClaimFloor::StartClaimFloor()
{
    mClaimTimer.Start(1.0f);

    Animator& animator = GetCreature().GetAnimator();
    animator.ChangeState(CreatureAnimConst::StateClaimFloor);
    animator.SetParamValue(CreatureAnimConst::ParamIsClaimFloor, true);
}

void CreatureAction_ClaimFloor::StopClaimFloor()
{
    Animator& animator = GetCreature().GetAnimator();
    animator.SetParamValue(CreatureAnimConst::ParamIsClaimFloor, false);
}

bool CreatureAction_ClaimFloor::ProcessClaimFloor(float stepDeltaTime)
{
    if (!mClaimTimer.IsOngoing())
        return false;

    cxx_assert(mFloorTile);

    if (mClaimTimer.TickAndCheckExpire(stepDeltaTime))
    {
        if (!gGameWorld.ClaimTile(mFloorTile, GetCreature().GetOwnerId()) ||
            !gGameWorld.CanClaimTile(mFloorTile, GetCreature().GetOwnerId()))
        {
            return false;
        }
        // continue with operation
        mClaimTimer.Start();
    }

    return true;
}
