#include "stdafx.h"
#include "CreatureAction_CarryGoldToTreasury.h"
#include "MapTile.h"
#include "Room.h"
#include "Creature.h"
#include "NavigationService.h"
#include "CreatureManager.h"
#include "GameMap.h"

CreatureAction_CarryGoldToTreasury::CreatureAction_CarryGoldToTreasury()
    : CreatureAction(eCreatureAction_CarryGoldToTreasury)
{
}

void CreatureAction_CarryGoldToTreasury::Configure(Creature* creature, const MapPoint2D& targetTile)
{
    CreatureAction::Configure(creature);
    mTargetMapTile = gGameMap.GetMapTileOrNull(targetTile);
    cxx_assert(mTargetMapTile);
}

void CreatureAction_CarryGoldToTreasury::OnRecycle()
{
    CreatureAction::OnRecycle();
    mTargetMapTile = {};
}

void CreatureAction_CarryGoldToTreasury::HandleEnterAction()
{
    cxx_assert(mTargetMapTile);
    if (mTargetMapTile)
    {
        const MapPoint2D creatureTile = GetCreature().GetTilePosition();
        Room* targetRoom = mTargetMapTile->mRoomInstance;
        glm::vec2 destinationPoint;
        bool isSuccess = targetRoom && targetRoom->GetOwnerId() == GetCreature().GetOwnerId() &&
            gNavigationService.CheckPathExists(creatureTile, mTargetMapTile->mLocation, GetCreature().GetPassabilityType()) &&
            gNavigationService.GetRandomPointWithinTile(mTargetMapTile->mLocation, destinationPoint);

        cxx_assert(isSuccess);
        if (isSuccess)
        {
            // go for it
            StartSubAction(gCreatureManager.CreateWalkToPointAction(GetCreaturePtr(), destinationPoint));
            return;
        }
    }

    SetActionResult(CreatureAction::eResult_Failed);
}

void CreatureAction_CarryGoldToTreasury::HandleLeaveAction()
{

}

void CreatureAction_CarryGoldToTreasury::HandleUpdateLogic(float stepDeltaTime)
{
    cxx_assert(false);
    SetActionResult(CreatureAction::eResult_Failed);
}

void CreatureAction_CarryGoldToTreasury::HandleResumeAction(eCreatureAction subActionId, eResult subActionResult)
{
    if (subActionId == eCreatureAction_WalkToPoint)
    {
        if (subActionResult != CreatureAction::eResult_Success)
        {
            SetActionResult(subActionResult);
            return;
        }

        // try store gold
        const MapPoint2D tileCoord = GetCreature().GetTilePosition();

        MapTile* mapTile = gGameMap.GetMapTileOrNull(tileCoord);
        if (mapTile == nullptr)
        {
            cxx_assert(false);
            SetActionResult(CreatureAction::eResult_Failed);
            return;
        }

        if ((mapTile->mRoomInstance == nullptr) || 
            (mapTile->mRoomInstance->GetOwnerId() != GetCreature().GetOwnerId()))
        {
            cxx_assert(false);
            SetActionResult(CreatureAction::eResult_Failed);
            return;
        }

        auto* moneyStorage = mapTile->mRoomInstance->GetCapability<MoneyStorageRoomCapability>();
        cxx_assert(moneyStorage);
        if (moneyStorage == nullptr)
        {
            SetActionResult(CreatureAction::eResult_Failed);
            return;
        }

        long storedAmount = moneyStorage->StoreGold(GetCreature().GetMoneyCarried(), tileCoord);
        if (storedAmount > 0)
        {
            GetCreature().WithdrawMoney(storedAmount);
            SetActionResult(CreatureAction::eResult_Success);
            return;
        }
        else
        {
            SetActionResult(CreatureAction::eResult_Failed);
            return;
        }

        return;
    }

    cxx_assert(false);
}

