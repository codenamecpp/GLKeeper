#pragma once

//////////////////////////////////////////////////////////////////////////

#include "RoomController.h"

//////////////////////////////////////////////////////////////////////////

class TortureChamberRoomController: public RoomController
{
public:
    TortureChamberRoomController() = default;

    // override RoomController
    void SpawnInstance() override;
    void DespawnInstance() override;
    void PostRearrangeObjects() override;
    void PostReconfigureRoom() override;
    void EvaluateFloorFurniture(FurnitureEvaluationResult& evaluation) override;
    void EvaluateWallFurniture(FurnitureEvaluationResult& evaluation) override;
};