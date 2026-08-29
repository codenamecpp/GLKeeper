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
#include "Scene.h"
#include "MapUtils.h"

//////////////////////////////////////////////////////////////////////////

#define FRONT_END_BEAMS_EFFECT_MESH "3dfe_beams"
#define FRONT_END_LEVELS_TABLE_MESH "3dmap_level21"

//////////////////////////////////////////////////////////////////////////

void HeroGateFrontendRoomController::PostReconfigureRoom()
{
    Init3dMap();
}

void HeroGateFrontendRoomController::SpawnInstance()
{
    RoomController::SpawnInstance();
}

void HeroGateFrontendRoomController::DespawnInstance()
{
    RoomController::DespawnInstance();
    m3dMapTable.reset();
    m3dMapTablePieces.clear();
}

void HeroGateFrontendRoomController::OnRecycle()
{
    RoomController::OnRecycle();
    m3dMapTable.reset();
    m3dMapTablePieces.clear();
}

void HeroGateFrontendRoomController::EvaluateFloorFurniture(RoomFurnitureSlots& evaluation)
{
    // TODO : temporary implementation

    const Rect2D& locationArea = GetRoom().GetLocationArea();

    Point2D baseLocation {locationArea.x, locationArea.y};

    auto PutObject = [&evaluation](const Point2D& tileLocation, GameObjectClassId objectId)
        {
            RoomFurnitureSlot& objectSlot = evaluation.emplace_back();
            objectSlot.mObjectClassId = objectId;
            objectSlot.mTileLocation = tileLocation;
        };

    auto PutCandle = [&evaluation](const Point2D& tileLocation, RoomFurnitureSlot::eFaceRotation rotation)
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
    GameObjectManager& gobjects = gGameObjectManager;
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

void HeroGateFrontendRoomController::Init3dMap()
{
    if (m3dMapTable)
        return;

    const Rect2D& locationArea = GetRoom().GetLocationArea();

    Point2D tileLocation {locationArea.x + 1, locationArea.y + 3};
    glm::vec3 tablePosition = MapUtils::ComputeTileCenter(tileLocation);

    MeshAsset* tableMesh = gMeshAssetManager.GetMesh(FRONT_END_LEVELS_TABLE_MESH);

    m3dMapTable = gScene.CreateAnimatingMesh();
    m3dMapTable->Configure(tableMesh);
    m3dMapTable->SetPosition(tablePosition);
    m3dMapTable->SetObjectActive(true);

    // todo: refactore

    const std::string pieces[] =
    {
        "3dmap_level1",
        "3dmap_level2",
        "3dmap_level3",
        "3dmap_level4",
        "3dmap_level5",
        "3dmap_level6",
        "3dmap_level6a",
        "3dmap_level6b",
        "3dmap_level7",
        "3dmap_level8",
        "3dmap_level9",
        "3dmap_level10",
        "3dmap_level11",
        "3dmap_level11a",
        "3dmap_level11b",
        "3dmap_level11c",
        "3dmap_level12",
        "3dmap_level13",
        "3dmap_level14",
        "3dmap_level15",
        "3dmap_level15a",
        "3dmap_level15b",
        "3dmap_level16",
        "3dmap_level17",
        "3dmap_level18",
        "3dmap_level19",
        "3dmap_level20"
    };

    for (const std::string& roller: pieces)
    {
        auto& piecePtr = m3dMapTablePieces.emplace_back();

        MeshAsset* pieceMesh = gMeshAssetManager.GetMesh(roller);

        piecePtr = gScene.CreateAnimatingMesh();
        piecePtr->Configure(pieceMesh);
        piecePtr->SetPosition(tablePosition);
        piecePtr->SetObjectActive(true);
    }
}
