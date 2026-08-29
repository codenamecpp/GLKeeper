#include "stdafx.h"
#include "DungeonHeartRoomController.h"
#include "TileConstructor.h"
#include "GameObject.h"
#include "MeshAssetManager.h"
#include "GameWorld.h"
#include "GameMain.h"
#include "GameObjectManager.h"
#include "GameSession.h"

DungeonHeartRoomController::DungeonHeartRoomController()
    : MoneyStorageRoomController()
{
}

void DungeonHeartRoomController::ConfigureInstance(Room* roomInstance)
{
    MoneyStorageRoomController::ConfigureInstance(roomInstance);

    const ScenarioVariables& vars = gGameSession.GetScenarioVariables();
    SetMoneyStorageMaxGoldPerTile(vars.mMaxGoldPerDungeonHeartTile);
}

void DungeonHeartRoomController::SpawnInstance()
{
    MoneyStorageRoomController::SpawnInstance();

}

void DungeonHeartRoomController::DespawnInstance()
{
    MoneyStorageRoomController::DespawnInstance();
}

void DungeonHeartRoomController::EvaluateFloorFurniture(RoomFurnitureSlots& evaluation)
{
    const Rect2D& locationArea = GetRoom().GetLocationArea();

    RoomFurnitureSlot& objectSlot = evaluation.emplace_back();
    objectSlot.mObjectClassId = GameObjectClassId_DungeonHeart;
    objectSlot.mTileLocation = {locationArea.x + 2, locationArea.y + 2};
}

void DungeonHeartRoomController::PostRearrangeObjects()
{
    MoneyStorageRoomController::PostRearrangeObjects();

    GameObjectManager& gobjects = gGameObjectManager;
    for (const RoomFurnitureSlot& roller: GetRoom().GetFloorFurniture())
    {
        if (roller.mObjectClassId != GameObjectClassId_DungeonHeart) 
            continue;

        if (GameObject* gameObject = gobjects.GetObjectPtr(roller.mObjectHandle))
        {
            if (AnimatingMeshObject* meshObject = gameObject->GetMeshObject())
            {
                glm::vec3 position = meshObject->GetPosition();
                position.y = MAP_TILE_HALF_SIZE;
                meshObject->SetPosition(position);
            }
        }
    }
}

void DungeonHeartRoomController::PostReconfigureRoom()
{
    MoneyStorageRoomController::PostReconfigureRoom();
}

void DungeonHeartRoomController::OnRecycle()
{
    MoneyStorageRoomController::OnRecycle();
}

void DungeonHeartRoomController::EvaluateStorageTiles(cxx::any_vector<Point2D> evaluationResult) const
{
    evaluationResult.reserve(16);

    for (MapTile* floorTile: GetRoom().GetFloorTiles())
    {
        if (floorTile->mIsRoomInnerTile) 
            continue;

        evaluationResult.push_back(floorTile->mLocation);
    }
}
