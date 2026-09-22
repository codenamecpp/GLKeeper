#include "stdafx.h"
#include "RoomController.h"
#include "GameWorld.h"

RoomController::~RoomController()
{

}

void RoomController::ConfigureInstance(Room* roomInstance)
{
    cxx_assert(mRoom == nullptr);
    mRoom = roomInstance;
    cxx_assert(mRoom);
}

void RoomController::SpawnInstance()
{

}

void RoomController::DespawnInstance()
{

}

void RoomController::UpdateLogic(float stepDeltaTime)
{

}

void RoomController::EvaluateFloorFurniture(RoomFurnitureSlots& evaluation)
{

}

void RoomController::EvaluateWallFurniture(RoomFurnitureSlots& evaluation)
{

}

void RoomController::EvaluatePillars(RoomFurnitureSlots& evaluation)
{
    if (GetRoomPillarObjectId() == GameObjectClassId_Null) 
        return;

    for (MapTile* currentTile: mPillarTiles)
    {
        RoomFurnitureSlot& objectSlot = evaluation.emplace_back();
        objectSlot.mTileLocation = currentTile->mLocation;
        objectSlot.mObjectClassId = GetRoomPillarObjectId();
        // rotate
        // 1. top-left / top-right corner
        if (currentTile->SameNeighbourRoomInstance(eDirection_S))
        {
            // E or W
            objectSlot.mObjectRotation = currentTile->SameNeighbourRoomInstance(eDirection_E) ? 
                RoomFurnitureSlot::eFaceRotation_0 : 
                RoomFurnitureSlot::eFaceRotation_90_Neg;
        }
        else // bottom left / bottom right corner
        {
            // E or W
            objectSlot.mObjectRotation = currentTile->SameNeighbourRoomInstance(eDirection_E) ? 
                RoomFurnitureSlot::eFaceRotation_90_Pos : 
                RoomFurnitureSlot::eFaceRotation_180;
        }
    }
}

void RoomController::PostRearrangeObjects()
{

}

void RoomController::PostReconfigureRoom()
{
    ReevaluatePillarTiles();
}

void RoomController::RoomOwnershipChanged(ePlayerID previousOwnerId, ePlayerID ownerId)
{

}

void RoomController::OnRecycle()
{
    DespawnInstance();
    mRoom = nullptr;
    mPillarTiles.clear();
}

void RoomController::ReevaluatePillarTiles()
{
    mPillarTiles.clear();

    if (GetRoomPillarObjectId() == GameObjectClassId_Null)
        return;

    // pattern for normal pillars:
    // it is must be placed in the corners or 3x3 room square

    auto CheckForRoomCorner = [](MapTile* tile)
        {
            int neighbours = tile->SameNeighbourRoomInstance(eDirection_N) + 
                tile->SameNeighbourRoomInstance(eDirection_E) +
                tile->SameNeighbourRoomInstance(eDirection_S) + 
                tile->SameNeighbourRoomInstance(eDirection_W);
            return (neighbours == 2);
        };

    for (MapTile* mapTile: mRoom->GetInnerTiles())
    {
        // check for corners
        for (eDirection dir: gDiagonalDirections)
        {
            MapTile* neighbourTile = mapTile->mNeighbours[dir];
            if (cxx::contains(mPillarTiles, neighbourTile))
                continue;

            if (CheckForRoomCorner(neighbourTile))
            {
                mPillarTiles.push_back(neighbourTile);
                gGameWorld.InvalidateTile(neighbourTile);
            }
        }
    }
}
