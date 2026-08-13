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
    void EvaluateFloorFurniture(RoomFurnitureSlots& evaluation) override;
    void EvaluateWallFurniture(RoomFurnitureSlots& evaluation) override;
    void EvaluatePillars(RoomFurnitureSlots& evaluation) override;
    void PostRearrangeObjects() override;
};
