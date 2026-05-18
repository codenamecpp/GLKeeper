#include "stdafx.h"
#include "HeroGateFrontendRoomController.h"
#include "MapTile.h"
#include "GameObject.h"
#include "GameMain.h"
#include "GameWorldDefs.h"
#include "GameWorld.h"
#include "AnimatingMeshObject.h"
#include "MeshAssetManager.h"
#include "GameObjectManager.h"

//////////////////////////////////////////////////////////////////////////

#define FRONT_END_BEAMS_EFFECT_MESH "3dfe_beams"
#define FRONT_END_LEVELS_TABLE_MESH "3dmap_level21"

//////////////////////////////////////////////////////////////////////////

void HeroGateFrontendRoomController::PostReconfigureRoom()
{
    if (mCampaignLevelsTable == nullptr)
    {
        // TODO : 

        const MapArea2D& locationArea = GetRoom().GetLocationArea();

        MapPoint2D tileLocation {locationArea.x + 1, locationArea.y + 3};
        glm::vec3 tablePosition = MapUtils::ComputeTileCenter(tileLocation);

        MeshAsset* tableMesh = gMeshAssetManager.GetMesh(FRONT_END_LEVELS_TABLE_MESH);

        mCampaignLevelsTable = GetScene().CreateAnimatingMesh();
        mCampaignLevelsTable->Configure(tableMesh);
        mCampaignLevelsTable->SetPosition(tablePosition);
        mCampaignLevelsTable->SetObjectActive(true);
    }
}

void HeroGateFrontendRoomController::SpawnInstance()
{
    RoomController::SpawnInstance();
}

void HeroGateFrontendRoomController::DespawnInstance()
{
    RoomController::DespawnInstance();
    mCampaignLevelsTable.reset();
}

void HeroGateFrontendRoomController::EvaluateFloorFurniture(FurnitureEvaluationResult& evaluation)
{
    // TODO : temporary implementation

    const MapArea2D& locationArea = GetRoom().GetLocationArea();

    MapPoint2D baseLocation {locationArea.x, locationArea.y};

    auto PutObject = [&evaluation](const MapPoint2D& tileLocation, GameObjectClassId objectId)
        {
            RoomFurnitureSlot& objectSlot = evaluation.emplace_back();
            objectSlot.mObjectClassId = objectId;
            objectSlot.mTileLocation = tileLocation;
        };

    auto PutCandle = [&evaluation](const MapPoint2D& tileLocation, RoomFurnitureSlot::eFaceRotation rotation)
        {
            RoomFurnitureSlot& objectSlot = evaluation.emplace_back();
            objectSlot.mObjectClassId = GameObjectClassId_3DFrontEndChain;
            objectSlot.mObjectRotation = rotation;
            objectSlot.mTileLocation = tileLocation;
        };

    PutObject({baseLocation.x + 1, baseLocation.y}, GameObjectClassId_3DFrontEndBanner1);
    PutObject({baseLocation.x + 1, baseLocation.y}, GameObjectClassId_3DFrontEndBanner2);
    PutCandle({baseLocation.x + 1, baseLocation.y}, RoomFurnitureSlot::eFaceRotation_0);
    PutCandle({baseLocation.x + 1, baseLocation.y + 1}, RoomFurnitureSlot::eFaceRotation_180);

    PutObject({baseLocation.x + 1, baseLocation.y + 1}, GameObjectClassId_3DFrontEndBanner3);
    PutObject({baseLocation.x + 1, baseLocation.y + 1}, GameObjectClassId_3DFrontEndBanner4);
    PutCandle({baseLocation.x + 1, baseLocation.y + 1}, RoomFurnitureSlot::eFaceRotation_0);
    PutCandle({baseLocation.x + 1, baseLocation.y + 2}, RoomFurnitureSlot::eFaceRotation_180);

    PutObject({baseLocation.x + 1, baseLocation.y + 2}, GameObjectClassId_3DFrontEndBanner1);
    PutObject({baseLocation.x + 1, baseLocation.y + 2}, GameObjectClassId_3DFrontEndBanner2);
    PutCandle({baseLocation.x + 1, baseLocation.y + 2}, RoomFurnitureSlot::eFaceRotation_0);
    PutCandle({baseLocation.x + 1, baseLocation.y + 3}, RoomFurnitureSlot::eFaceRotation_180);

    PutObject({baseLocation.x + 1, baseLocation.y + 3}, GameObjectClassId_3DFrontEndBanner3);
    PutObject({baseLocation.x + 1, baseLocation.y + 3}, GameObjectClassId_3DFrontEndBanner4);
    PutCandle({baseLocation.x + 1, baseLocation.y + 3}, RoomFurnitureSlot::eFaceRotation_0);
    PutCandle({baseLocation.x + 1, baseLocation.y + 4}, RoomFurnitureSlot::eFaceRotation_180);

    PutObject({baseLocation.x + 1, baseLocation.y}, GameObjectClassId_3DFrontEndGemHolder);
}

void HeroGateFrontendRoomController::PostRearrangeObjects()
{
    GameObjectManager& gobjects = GetObjectManager();
    for (const RoomFurnitureSlot& roller: GetRoom().GetFloorFurniture())
    {
        if (GameObject* gameObject = gobjects.GetObjectPtr(roller.mObjectHandle))
        {
            if (AnimatingMeshObject* meshObject = gameObject->GetMeshObject())
            {
                meshObject->SetAnimationProgress(Random::GenerateFloat01());
            }
        }
    }
}
