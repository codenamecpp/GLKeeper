#include "stdafx.h"
#include "TrainingRoomController.h"

void TrainingRoomController::SpawnInstance()
{
    RoomController::SpawnInstance();
}

void TrainingRoomController::DespawnInstance()
{
    RoomController::DespawnInstance();
}

void TrainingRoomController::PostRearrangeObjects()
{
    RoomController::PostRearrangeObjects();
}

void TrainingRoomController::PostReconfigureRoom()
{
    RoomController::PostReconfigureRoom();
}

void TrainingRoomController::EvaluateFloorFurniture(FurnitureEvaluationResult& evaluation)
{
    RoomDefinition* roomDefinition = GetRoom().GetDefinition();
    GameObjectClassId objectIds[] =
    { 
        roomDefinition->mObjectIds[0], 
        (roomDefinition->mObjectIds[1] == GameObjectClassId_Null) ? roomDefinition->mObjectIds[0] : roomDefinition->mObjectIds[1], 
        (roomDefinition->mObjectIds[2] == GameObjectClassId_Null) ? roomDefinition->mObjectIds[0] : roomDefinition->mObjectIds[2]
    };

    for (MapTile* mapTile: GetRoom().GetInnerTiles())
    {
        int objectIndex = mapTile->mRandomValue % CountOf(objectIds); // quick and dirty


        RoomFurnitureSlot& roomObject = evaluation.emplace_back();
        roomObject.mObjectClassId = objectIds[objectIndex];
        roomObject.mTileLocation = mapTile->mTileLocation;
        roomObject.mObjectRotation = RoomFurnitureSlot::eFaceRotation_Random;
    }
}

void TrainingRoomController::EvaluateWallFurniture(FurnitureEvaluationResult& evaluation)
{
    RoomDefinition* roomDefinition = GetRoom().GetDefinition();
    GameObjectClassId objectIds[] =
    { 
        roomDefinition->mObjectIds[3], 
        (roomDefinition->mObjectIds[4] == GameObjectClassId_Null) ? roomDefinition->mObjectIds[3] : roomDefinition->mObjectIds[4], 
        (roomDefinition->mObjectIds[5] == GameObjectClassId_Null) ? roomDefinition->mObjectIds[3] : roomDefinition->mObjectIds[5]
    };

    for (const RoomWallSection* wallSection: GetRoom().GetWallSections())
    {
        for (MapTile* wallTile: wallSection->GetTiles())
        {
            if (!wallSection->IsInner(wallTile) || wallSection->IsOdd(wallTile)) 
                continue;

            eDirection wallFaceDirection = wallSection->GetDirection();

            int objectIndex = wallTile->mRandomValue % CountOf(objectIds); // quick and dirty

            RoomFurnitureSlot& roomObject = evaluation.emplace_back();
            roomObject.mObjectClassId = objectIds[objectIndex];
            roomObject.mTileLocation = wallTile->mNeighbours[wallFaceDirection]->mTileLocation;
            switch (wallFaceDirection)
            {
                case eDirection_N: 
                    roomObject.mObjectRotation = RoomFurnitureSlot::eFaceRotation_180; 
                    roomObject.mObjectPositioning = RoomFurnitureSlot::ePositioning_WallS; // opposite to wall face
                break;
                case eDirection_E: 
                    roomObject.mObjectRotation = RoomFurnitureSlot::eFaceRotation_90_Pos; 
                    roomObject.mObjectPositioning = RoomFurnitureSlot::ePositioning_WallW; // opposite to wall face
                break;
                case eDirection_S:
                    roomObject.mObjectPositioning = RoomFurnitureSlot::ePositioning_WallN; // opposite to wall face
                break;
                case eDirection_W: 
                    roomObject.mObjectRotation = RoomFurnitureSlot::eFaceRotation_90_Neg; 
                    roomObject.mObjectPositioning = RoomFurnitureSlot::ePositioning_WallE; // opposite to wall face
                break;
            }
        }
    }
}
