#pragma once

//////////////////////////////////////////////////////////////////////////

#include "RoomController.h"
#include "AnimatingMeshObject.h"

//////////////////////////////////////////////////////////////////////////

// Hero Gate FrontEnd Room Logic
class HeroGateFrontendRoomController: public RoomController
{
public:
    HeroGateFrontendRoomController() = default;

    // override RoomController
    void SpawnInstance() override;
    void DespawnInstance() override;
    void EvaluateFloorFurniture(FurnitureEvaluationResult& evaluation) override;
    void PostReconfigureRoom() override;
    void PostRearrangeObjects() override;

private:
    cxx::uniqueptr<AnimatingMeshObject> mCampaignLevelsTable;
};