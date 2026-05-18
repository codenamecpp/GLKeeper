#pragma once

//////////////////////////////////////////////////////////////////////////

#include "RoomController.h"

//////////////////////////////////////////////////////////////////////////

class WorkshopRoomController: public RoomController
{
public:
    WorkshopRoomController() = default;

    // override RoomController
    void SpawnInstance() override;
    void DespawnInstance() override;
    void EvaluateFloorFurniture(FurnitureEvaluationResult& evaluation) override;
    void EvaluateWallFurniture(FurnitureEvaluationResult& evaluation) override;
    void EvaluatePillars(FurnitureEvaluationResult& evaluation) override;
    void PostRearrangeObjects() override;
};
