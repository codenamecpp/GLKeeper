#include "stdafx.h"
#include "RoomTileConstructor_Quad.h"
#include "MeshAssetManager.h"
#include "Room.h"
#include "TileConstructor.h"

RoomTileConstructor_Quad::RoomTileConstructor_Quad(TileConstructor& baseConstructor)
    : RoomTileConstructor(baseConstructor)
{
}

void RoomTileConstructor_Quad::ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles)
{
    cxx_assert(roomInstance);

    RoomDefinition* definition = roomInstance->GetDefinition();

    const std::string meshName = definition->mCompleteResource.mResourceName;

    // rotation shortcuts
    const glm::mat3* Neg90 = &g_TileRotations[0];
    const glm::mat3* Pos90 = &g_TileRotations[1];
    const glm::mat3* NegPi = &g_TileRotations[2];
    const glm::mat3* Nullp = nullptr;

    static const glm::mat3* subtileRotations[][8] = {
        {Pos90, Pos90, Pos90, Pos90, NegPi, Neg90, NegPi, Pos90},
        {Nullp, Nullp, Nullp, Nullp, Pos90, NegPi, Pos90, Nullp},
        {Neg90, Neg90, Neg90, Neg90, Nullp, Pos90, Nullp, Neg90},
        {NegPi, NegPi, NegPi, NegPi, Neg90, Nullp, Neg90, NegPi},
    };

    MeshAsset* geoPiece0 = gMeshAssetManager.GetMesh(meshName + "0");
    MeshAsset* geoPiece1 = gMeshAssetManager.GetMesh(meshName + "1");
    MeshAsset* geoPiece2 = gMeshAssetManager.GetMesh(meshName + "2");

    MeshAsset* pieces[8] = 
    {
        geoPiece1, geoPiece0, geoPiece1, geoPiece0,
        geoPiece0, geoPiece2, geoPiece0, geoPiece0
    };

    for (MapTile* targetTile : floorTiles)
    {
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
                subtileRotations[isubtile][subTiles[isubtile]], &g_SubTileTranslations[isubtile]);
        }
    }
}
