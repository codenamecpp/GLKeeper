#pragma once

//////////////////////////////////////////////////////////////////////////

#include "MoneyStorageRoomController.h"

//////////////////////////////////////////////////////////////////////////

class TreasuryRoomController: public MoneyStorageRoomController
{
public:
    TreasuryRoomController();

    // override MoneyStorageRoomController
    void ConfigureInstance(Room* roomInstance) override;
    void SpawnInstance() override;
    void DespawnInstance() override;
    void PostRearrangeObjects() override;
    void PostReconfigureRoom() override;

    // pool
    void OnRecycle() override;

private:
    // override StorageRoomController
    void EvaluateStorageTiles(StorageTilesEvaluationResult& evaluationResult) const override;
};

//////////////////////////////////////////////////////////////////////////
