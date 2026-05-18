#include "stdafx.h"
#include "TortureChamberRoomController.h"
#include "GameObject.h"
#include "SceneObject.h"

void TortureChamberRoomController::SpawnInstance()
{
    RoomController::SpawnInstance();
}

void TortureChamberRoomController::DespawnInstance()
{
    RoomController::DespawnInstance();
}

void TortureChamberRoomController::PostRearrangeObjects()
{
    RoomController::PostRearrangeObjects();

}

void TortureChamberRoomController::PostReconfigureRoom()
{
    RoomController::PostReconfigureRoom();


}

void TortureChamberRoomController::EvaluateFloorFurniture(FurnitureEvaluationResult& evaluation)
{

}

void TortureChamberRoomController::EvaluateWallFurniture(FurnitureEvaluationResult& evaluation)
{
    RoomDefinition* roomDefinition = GetRoom().GetDefinition();
    for (const RoomWallSection* wallSection: GetRoom().GetWallSections())
    {
        eDirection wallFaceDirection = wallSection->GetDirection();
        for (MapTile* wallTile: wallSection->GetTiles())
        {
            if (!wallSection->IsInner(wallTile) || wallSection->IsOdd(wallTile)) 
                continue;

            RoomFurnitureSlot& roomObject = evaluation.emplace_back();
            roomObject.mObjectClassId = roomDefinition->mObjectIds[4];
            roomObject.mTileLocation = wallTile->mNeighbours[wallFaceDirection]->mTileLocation;
            switch (wallFaceDirection)
            {
                case eDirection_N: 
                    roomObject.mObjectRotation = RoomFurnitureSlot::eFaceRotation_180;
                    roomObject.mObjectPositioning = RoomFurnitureSlot::ePositioning_WallS; // opposite no wall face
                break;
                case eDirection_E: 
                    roomObject.mObjectRotation = RoomFurnitureSlot::eFaceRotation_90_Pos; 
                    roomObject.mObjectPositioning = RoomFurnitureSlot::ePositioning_WallW; // opposite no wall face
                break;
                case eDirection_S:
                    roomObject.mObjectPositioning = RoomFurnitureSlot::ePositioning_WallN; // opposite no wall face
                break;
                case eDirection_W: roomObject.mObjectRotation = RoomFurnitureSlot::eFaceRotation_90_Neg; 
                    roomObject.mObjectPositioning = RoomFurnitureSlot::ePositioning_WallE; // opposite no wall face
                break;
            }
        }
    }
}
