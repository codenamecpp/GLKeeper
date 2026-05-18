#include "stdafx.h"
#include "LibraryRoomController.h"

void LibraryRoomController::SpawnInstance()
{
    RoomController::SpawnInstance();
}

void LibraryRoomController::DespawnInstance()
{
    RoomController::DespawnInstance();
}

void LibraryRoomController::PostRearrangeObjects()
{
    RoomController::PostRearrangeObjects();
}

void LibraryRoomController::PostReconfigureRoom()
{
    RoomController::PostReconfigureRoom();
}

void LibraryRoomController::EvaluateFloorFurniture(FurnitureEvaluationResult& evaluation)
{
    RoomDefinition* roomDefinition = GetRoom().GetDefinition();

    const int FirstObjectIndex = 0;
    const int MaxObjectIds = 3;

    for (MapTile* mapTile: GetRoom().GetInnerTiles())
    {
        int objectIndex = mapTile->mRandomValue % MaxObjectIds; // quick and dirty

        RoomFurnitureSlot& roomObject = evaluation.emplace_back();
        roomObject.mObjectClassId = roomDefinition->mObjectIds[FirstObjectIndex + objectIndex];
        if (roomObject.mObjectClassId == GameObjectClassId_Null)
        {
            roomObject.mObjectClassId = roomDefinition->mObjectIds[FirstObjectIndex];
        }
        roomObject.mTileLocation = mapTile->mTileLocation;
        roomObject.mObjectRotation = cxx::is_even(mapTile->mTileLocation.y) ? 
            RoomFurnitureSlot::eFaceRotation_180 : 
            RoomFurnitureSlot::eFaceRotation_0;

        // select variation
        eGameObjectMeshId meshId = static_cast<eGameObjectMeshId>(1 + (mapTile->mRandomValue % 3)); // todo: fix
        roomObject.mObjectMeshId = meshId;
    }
}

void LibraryRoomController::EvaluateWallFurniture(FurnitureEvaluationResult& evaluation)
{
    RoomDefinition* roomDefinition = GetRoom().GetDefinition();

    const int FirstObjectIndex = 3;
    const int MaxObjectIds = 3;

    for (const RoomWallSection* wallSection: GetRoom().GetWallSections())
    {
        for (MapTile* wallTile: wallSection->GetTiles())
        {
            if (!wallSection->IsInner(wallTile) || wallSection->IsOdd(wallTile)) 
                continue;

            eDirection wallFaceDirection = wallSection->GetDirection();

            int objectIndex = wallTile->mRandomValue % MaxObjectIds; // quick and dirty

            RoomFurnitureSlot& roomObject = evaluation.emplace_back();
            roomObject.mObjectClassId = roomDefinition->mObjectIds[FirstObjectIndex + objectIndex];
            if (roomObject.mObjectClassId == GameObjectClassId_Null)
            {
                roomObject.mObjectClassId = roomDefinition->mObjectIds[FirstObjectIndex];
            }
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
