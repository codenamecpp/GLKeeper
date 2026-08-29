#pragma once

//////////////////////////////////////////////////////////////////////////

#include "RoomController.h"
#include "EnvironmentMeshObject.h"

//////////////////////////////////////////////////////////////////////////

// Temple Room Logic
class TempleRoomController: public RoomController
{
public:
    TempleRoomController() = default;

    // override RoomController
    void SpawnInstance() override;
    void DespawnInstance() override;
    void PostReconfigureRoom() override;
    void EvaluateFloorFurniture(RoomFurnitureSlots& evaluation) override;
    void EvaluatePillars(RoomFurnitureSlots& evaluation) override;

private:
    void ReevaluateWaterPoolTiles();
    bool ReevaluateHandLocation();
    void DestroyWaterPool();

private:
    cxx::uniqueptr<EnvironmentMeshObject> mWaterPool;
    std::vector<MapTile*> mWaterPoolTiles;
    std::optional<Point2D> mHandLocation;
};