#pragma once

//////////////////////////////////////////////////////////////////////////

#include "RoomController.h"

//////////////////////////////////////////////////////////////////////////

class TrainingRoomController: public RoomController
{
public:
    TrainingRoomController() = default;

    // override RoomController
    void SpawnInstance() override;
    void DespawnInstance() override;
    void PostRearrangeObjects() override;
    void PostReconfigureRoom() override;
    void EvaluateFloorFurniture(FurnitureEvaluationResult& evaluation) override;
    void EvaluateWallFurniture(FurnitureEvaluationResult& evaluation) override;

protected:

};