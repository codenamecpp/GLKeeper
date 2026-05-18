#pragma once

//////////////////////////////////////////////////////////////////////////

#include "RoomController.h"

//////////////////////////////////////////////////////////////////////////

class HatcheryRoomController: public RoomController
{
public:
    HatcheryRoomController() = default;

    // override RoomController
    void SpawnInstance() override;
    void DespawnInstance() override;
    void UpdateLogic(float stepDeltaTime) override;
    void PostRearrangeObjects() override;
    void PostReconfigureRoom() override;
    void EvaluateFloorFurniture(FurnitureEvaluationResult& evaluation) override;

protected:
    bool TryGenerateEggWithingRoom();

protected:
    int mGameTicksSinceLastEggGeneration = 0;
};