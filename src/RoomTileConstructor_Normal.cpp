#include "stdafx.h"
#include "RoomTileConstructor_Normal.h"
#include "MeshAssetManager.h"
#include "Room.h"
#include "TileConstructor.h"

RoomTileConstructor_Normal::RoomTileConstructor_Normal(TileConstructor& baseConstructor)
    : RoomTileConstructor(baseConstructor)
{

}

void RoomTileConstructor_Normal::ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles)
{
    cxx_assert(roomInstance);

    RoomDefinition* definition = roomInstance->GetDefinition();

    const std::string meshName = definition->mCompleteResource.mResourceName;

    // geometries
    MeshAsset* geoInnerPart = gMeshAssetManager.GetMesh(meshName + "9");
    MeshAsset* geoPiece0 = gMeshAssetManager.GetMesh(meshName + "0");
    MeshAsset* geoPiece1 = gMeshAssetManager.GetMesh(meshName + "1");
    MeshAsset* geoPiece2 = gMeshAssetManager.GetMesh(meshName + "2");
    MeshAsset* geoPiece3 = gMeshAssetManager.GetMesh(meshName + "3"); // inner
    MeshAsset* pieces[] =
    {
        geoPiece1, // !SideA && !Corner && !SideB
        geoPiece0, // SideB
        geoPiece1, // Corner
        geoPiece0, // SideB && Corner
        geoPiece0, // SideA
        geoPiece2, // SideA && SideB
        geoPiece0, // SideA && Corner
        geoPiece3, // SideA && Corner && SideB
    };

    // rotation shortcuts
    const glm::mat3* Neg90 = &g_TileRotations[0];
    const glm::mat3* Pos90 = &g_TileRotations[1];
    const glm::mat3* NegPi = &g_TileRotations[2];
    const glm::mat3* Nullp = nullptr;

    const glm::mat3* pieceRotations[][8] = {
        {Pos90, Pos90, Pos90, Pos90, Nullp, Neg90, Nullp, Nullp},
        {Nullp, Nullp, Nullp, Nullp, Neg90, NegPi, Neg90, Neg90},
        {Neg90, Neg90, Neg90, Neg90, NegPi, Pos90, NegPi, Nullp},
        {NegPi, NegPi, NegPi, NegPi, Pos90, Nullp, Pos90, Pos90}
    };

    for (MapTile* targetTile : floorTiles) 
    {
        // todo: fix it
        if ((targetTile->mRoomInstance == roomInstance) && targetTile->mIsRoomInnerTile)
        {
            mBaseTileConstructor.ExtendTileFloor(targetTile, geoInnerPart, nullptr);
            continue;
        }

        int subTiles[] = 
        {
        //SubtTopLeft
            (targetTile->SameNeighbourRoomInstance(eDirection_W) ? 0x04 : 0) | 
            (targetTile->SameNeighbourRoomInstance(eDirection_NW) ? 0x02 : 0) | 
            (targetTile->SameNeighbourRoomInstance(eDirection_N) ? 0x01 : 0),
        //SubtTopRight
            (targetTile->SameNeighbourRoomInstance(eDirection_N) ? 0x04 : 0) | 
            (targetTile->SameNeighbourRoomInstance(eDirection_NE) ? 0x02 : 0) | 
            (targetTile->SameNeighbourRoomInstance(eDirection_E) ? 0x01 : 0),
        //SubtBottomRight
            (targetTile->SameNeighbourRoomInstance(eDirection_E) ? 0x04 : 0) | 
            (targetTile->SameNeighbourRoomInstance(eDirection_SE) ? 0x02 : 0) | 
            (targetTile->SameNeighbourRoomInstance(eDirection_S) ? 0x01 : 0),
        //SubtBottomLeft
            (targetTile->SameNeighbourRoomInstance(eDirection_S) ? 0x04 : 0) | 
            (targetTile->SameNeighbourRoomInstance(eDirection_SW) ? 0x02 : 0) | 
            (targetTile->SameNeighbourRoomInstance(eDirection_W) ? 0x01 : 0)
        };

        for (int isubtile = 0; isubtile < 4; ++isubtile)
        {
            mBaseTileConstructor.ExtendTileFloor(targetTile, 
                pieces[subTiles[isubtile]],
                pieceRotations[isubtile][subTiles[isubtile]], &g_SubTileTranslations[isubtile]);
        }
    }
}
