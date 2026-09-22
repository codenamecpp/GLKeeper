#include "stdafx.h"
#include "CreatureAction_Mining.h"
#include "CreatureManager.h"
#include "CreatureAnimConst.h"
#include "MapUtils.h"
#include "GameMap.h"
#include "GameWorld.h"
#include "GameObjectManager.h"
#include "QueryService.h"

CreatureAction_Mining::CreatureAction_Mining()
    : CreatureAction(eCreatureAction_Mining)
{
}

void CreatureAction_Mining::Configure(Creature* creature, const glm::vec2& workPoint, const Point2D& targetTile)
{
    CreatureAction::Configure(creature);
    mWorkPoint = workPoint;
    mTargetTile = targetTile;
}

void CreatureAction_Mining::OnRecycle()
{
    CreatureAction::OnRecycle();
    mWorkPoint = {};
    mTargetTile = {};
    mMineTimer = {};
}

void CreatureAction_Mining::HandleEnterAction()
{
    // start walk to destination
    StartSubAction(gCreatureManager.CreateWalkToPointAction(GetCreaturePtr(), mWorkPoint));
}

void CreatureAction_Mining::HandleLeaveAction()
{
    StopTileMining();
}

void CreatureAction_Mining::HandleUpdateLogic(float stepDeltaTime)
{
    bool isCancelled = IsCancellationRequested();
    if (isCancelled || !ProcessTileMining(stepDeltaTime))
    {
        StopTileMining();
    }

    Animator& animator = GetCreature().GetAnimator();
    if (animator.IsCurrentState(CreatureAnimConst::StateDigging) && !animator.IsCurrentStateStopped())
    {
        // continue with mining
        return;
    }

    SetActionResult(isCancelled ? CreatureAction::eResult_Cancelled : CreatureAction::eResult_Success);
}

void CreatureAction_Mining::HandleResumeAction(eCreatureAction subActionId, eResult subActionResult)
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
        StartTileMining();
        return;
    }

    cxx_assert(false);
}

void CreatureAction_Mining::StartTileMining()
{
    mMineTimer.Start(1.0f);

    Animator& animator = GetCreature().GetAnimator();
    animator.ChangeState(CreatureAnimConst::StateDigging);
    animator.SetParamValue(CreatureAnimConst::ParamIsDigging, true);
}

void CreatureAction_Mining::StopTileMining()
{
    Animator& animator = GetCreature().GetAnimator();
    animator.SetParamValue(CreatureAnimConst::ParamIsDigging, false);
}

bool CreatureAction_Mining::ProcessTileMining(float stepDeltaTime)
{
    if (!mMineTimer.IsOngoing())
        return false;
    
    MapTile* targetTile = gGameMap.GetMapTile(mTargetTile);
    cxx_assert(targetTile);
    if (targetTile == nullptr)
    {
        return false;
    }

    if (mMineTimer.TickAndCheckExpire(stepDeltaTime))
    {
        long goldMined = 0;

        if (!gGameWorld.MineBlock(targetTile, GetCreature().GetOwnerId(), goldMined))
            return false;

        cxx_assert(goldMined > 0);

        long leftoverGold = 0;
        GetCreature().ReceiveMoney(goldMined, leftoverGold);

        if (leftoverGold > 0)
        {
            // drop loose gold on the floor
            // todo: refactore
            EntityHandle looseGoldObject = gGameObjectManager.CreateGoldPile(leftoverGold);
            if (GameObject* gameObject = gGameObjectManager.GetObjectPtr(looseGoldObject))
            {
                glm::vec3 objectPosition = GetCreature().GetPosition();
                gameObject->SetPosition(objectPosition);
                gameObject->SnapPositionToFloor();
            }
            gGameObjectManager.ActivateObject(looseGoldObject);
        }

        // we're full?
        if (!GetCreature().CanCarryMoreMoney())
        {
            // gems are intifite, stop mining
            if (targetTile->IsImpenetrable())
                return false;

            // in case there are available storage for deposit, stop mining
            cxx::temp_vector<EntityHandle> roomEntities;
            if (gQueryService.QueryAccessibleMoneyStorageRoomsForDeposit(
                GetCreature().GetOwnerId(), 
                GetCreature().GetOwnHandle(), 1, roomEntities))
            {
                return false;
            }

            // can continue mining?
            if (!gGameWorld.CanMineBlock(targetTile, GetCreature().GetOwnerId()))
                return false;
        }

        mMineTimer.Start();
    }

    return true;
}
