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
    void EvaluateFloorFurniture(FurnitureEvaluationResult& evaluation) override;
    void EvaluatePillars(FurnitureEvaluationResult& evaluation) override;

private:
    void ReevaluateWaterPoolTiles();
    bool ReevaluateHandLocation();
    void DestroyWaterPool();

private:
    cxx::uniqueptr<EnvironmentMeshObject> mWaterPool;
    std::vector<MapTile*> mWaterPoolTiles;
    std::optional<MapPoint2D> mHandLocation;
};