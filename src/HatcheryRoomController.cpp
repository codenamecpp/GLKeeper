#include "stdafx.h"
#include "HatcheryRoomController.h"
#include "GameMain.h"
#include "MapUtils.h"
#include "GameWorld.h"
#include "GameObjectManager.h"
#include "GameMap.h"

void HatcheryRoomController::SpawnInstance()
{
    RoomController::SpawnInstance();
    mGameTicksSinceLastEggGeneration = 0;
}

void HatcheryRoomController::DespawnInstance()
{
    RoomController::DespawnInstance();
    mGameTicksSinceLastEggGeneration = 0;
}

void HatcheryRoomController::UpdateLogic(float stepDeltaTime)
{
    RoomController::UpdateLogic(stepDeltaTime);

    if (++mGameTicksSinceLastEggGeneration >= 16)
    {
        mGameTicksSinceLastEggGeneration = 0;
        TryGenerateEggWithingRoom();
    }
}

void HatcheryRoomController::PostRearrangeObjects()
{
    RoomController::PostRearrangeObjects();
}

void HatcheryRoomController::PostReconfigureRoom()
{
    RoomController::PostReconfigureRoom();
}

void HatcheryRoomController::EvaluateFloorFurniture(RoomFurnitureSlots& evaluation)
{
    RoomDefinition* roomDefinition = GetRoom().GetDefinition();

    for (MapTile* mapTile: GetRoom().GetInnerTiles())
    {
        bool canPutObjectOnTile = true;

        // check for nearby objects
        for (const RoomFurnitureSlot& evalSlots: evaluation)
        {
            if ((std::abs(evalSlots.mTileLocation.x - mapTile->mLocation.x) < 2) && 
                (std::abs(evalSlots.mTileLocation.y - mapTile->mLocation.y) < 2)) 
            {
                canPutObjectOnTile = false;
                continue;
            }
        }

        if (!canPutObjectOnTile) continue;

        RoomFurnitureSlot& roomObject = evaluation.emplace_back();
        roomObject.mObjectClassId = roomDefinition->mObjectIds[0];
        roomObject.mTileLocation = mapTile->mLocation;
        roomObject.mObjectRotation = (mapTile->mRandomValue % 100) > 30 ?
            RoomFurnitureSlot::eFaceRotation_90_Pos : 
            RoomFurnitureSlot::eFaceRotation_0;
    }
}

bool HatcheryRoomController::TryGenerateEggWithingRoom()
{
    int numRoomTiles = GetRoom().GetRoomSize();
    if (numRoomTiles < 1) return false;
    // select tile
    int tileIndex = Random::GenerateInt(0, numRoomTiles - 1);

    MapTile* randomRoomTile = GetRoom().GetFloorTiles()[tileIndex];

    // select random position within tile
    cxx::aabbox blockBounds = MapUtils::ComputeBlockBounds(randomRoomTile->mLocation);
    glm::vec2 eggPosition2d
    {
        blockBounds.get_center().x + ((Random::GenerateFloat01() * blockBounds.get_width()) - (blockBounds.get_width() * 0.5f)) * 0.8f,
        blockBounds.get_center().z + ((Random::GenerateFloat01() * blockBounds.get_depth()) - (blockBounds.get_depth() * 0.5f)) * 0.8f
    };
    glm::vec3 eggPosition3d 
    {
        eggPosition2d.x, gGameMap.GetFloorHeightAt(eggPosition2d),
        eggPosition2d.y
    };

    EntityHandle objectHandle = gGameObjectManager.CreateObject(GameObjectClassId_Egg);
    if (GameObject* gameObject = gGameObjectManager.GetObjectPtr(objectHandle))
    {
        gameObject->SetPosition(eggPosition3d);
        return gGameObjectManager.ActivateObject(objectHandle);
    }
    cxx_assert(false);
    return false;
}

