#include "stdafx.h"
#include "TreasuryRoomController.h"

TreasuryRoomController::TreasuryRoomController()
    : MoneyStorageRoomController()
{
}

void TreasuryRoomController::ConfigureInstance(Room* roomInstance)
{
    MoneyStorageRoomController::ConfigureInstance(roomInstance);

    const ScenarioVariables& vars = GetScenarioVariables();
    SetMoneyStorageMaxGoldPerTile(vars.mMaxGoldPerTreasuryTile);
}

void TreasuryRoomController::SpawnInstance()
{
    MoneyStorageRoomController::SpawnInstance();
}

void TreasuryRoomController::DespawnInstance()
{
    MoneyStorageRoomController::DespawnInstance();
}

void TreasuryRoomController::PostRearrangeObjects()
{
    MoneyStorageRoomController::PostRearrangeObjects();
}

void TreasuryRoomController::PostReconfigureRoom()
{
    MoneyStorageRoomController::PostReconfigureRoom();
}

void TreasuryRoomController::OnRecycle()
{
    MoneyStorageRoomController::OnRecycle();
}

void TreasuryRoomController::EvaluateStorageTiles(StorageTilesEvaluationResult& evaluationResult) const
{
    evaluationResult.clear();

    cxx::span<MapTile*> floorTilesSpan = GetRoom().GetFloorTiles();
    evaluationResult.reserve(floorTilesSpan.size());
    for (MapTile* rollerTile: floorTilesSpan)
    {
        evaluationResult.push_back(rollerTile->mTileLocation);
    }
}
