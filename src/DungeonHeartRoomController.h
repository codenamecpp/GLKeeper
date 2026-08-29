#pragma once

//////////////////////////////////////////////////////////////////////////

#include "MoneyStorageRoomController.h"

//////////////////////////////////////////////////////////////////////////

class DungeonHeartRoomController: public MoneyStorageRoomController
{
public:
    DungeonHeartRoomController();

    // override MoneyStorageRoomController
    void ConfigureInstance(Room* roomInstance) override;
    void SpawnInstance() override;
    void DespawnInstance() override;
    void EvaluateFloorFurniture(RoomFurnitureSlots& evaluation) override;
    void PostRearrangeObjects() override;
    void PostReconfigureRoom() override;

    // pool
    void OnRecycle() override;

private:
    // override StorageRoomController
    void EvaluateStorageTiles(cxx::any_vector<Point2D> evaluationResult) const override;
};

//////////////////////////////////////////////////////////////////////////