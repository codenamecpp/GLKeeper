#pragma once

//////////////////////////////////////////////////////////////////////////

#include "RoomController.h"
#include "AnimatingMeshObject.h"

//////////////////////////////////////////////////////////////////////////

// Hero Gate FrontEnd Room Logic
class HeroGateFrontendRoomController: public RoomController
    , protected I3DMapTableRoom
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

    // override I3DMapTableRoom
    bool IsLevel(EntityHandle objectHandle) const override;
    bool SelectLevel(EntityHandle objectHandle) override;
    void DeselectLevel() override;
    bool GetLevelKey(EntityHandle objectHandle, CampaignLevelKey& levelKey) const override;

protected:
    void Init3dMap();

private:
    using TablePieceEntry = std::pair<EntityHandle, CampaignLevelKey>;
    std::vector<TablePieceEntry> m3dMapTablePieces;
    EntityHandle mSelectedTablePiece {};
};

//////////////////////////////////////////////////////////////////////////