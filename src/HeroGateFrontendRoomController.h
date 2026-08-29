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
    void OnRecycle() override;
    void EvaluateFloorFurniture(RoomFurnitureSlots& evaluation) override;
    void PostReconfigureRoom() override;
    void PostRearrangeObjects() override;

protected:
    void Init3dMap();

private:
    cxx::uniqueptr<AnimatingMeshObject> m3dMapTable;
    std::vector<cxx::uniqueptr<AnimatingMeshObject>> m3dMapTablePieces;
};