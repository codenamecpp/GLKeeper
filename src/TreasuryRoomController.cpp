#include "stdafx.h"
#include "TreasuryRoomController.h"
#include "GameSession.h"

TreasuryRoomController::TreasuryRoomController()
    : MoneyStorageRoomController()
{
}

void TreasuryRoomController::ConfigureInstance(Room* roomInstance)
{
    MoneyStorageRoomController::ConfigureInstance(roomInstance);

    const ScenarioVariables& vars = gGameSession.GetScenarioVariables();
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

void TreasuryRoomController::EvaluateStorageTiles(cxx::any_vector<Point2D> evaluationResult) const
{
    cxx::span<MapTile*> floorTilesSpan = GetRoom().GetFloorTiles();

    evaluationResult.reserve(floorTilesSpan.size());
    for (MapTile* rollerTile: floorTilesSpan)
    {
        evaluationResult.push_back(rollerTile->mLocation);
    }
}
