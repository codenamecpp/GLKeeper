#include "stdafx.h"
#include "RoomTileConstructor_DoubleQuad.h"
#include "MeshAssetManager.h"
#include "Room.h"
#include "TileConstructor.h"

RoomTileConstructor_DoubleQuad::RoomTileConstructor_DoubleQuad(TileConstructor& baseConstructor)
    : RoomTileConstructor(baseConstructor)
{
}

void RoomTileConstructor_DoubleQuad::ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles)
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
    MeshAsset* innerGeoPiece0 = gMeshAssetManager.GetMesh(meshName + "10");
    MeshAsset* innerGeoPiece1 = gMeshAssetManager.GetMesh(meshName + "11");
    MeshAsset* innerGeoPiece2 = gMeshAssetManager.GetMesh(meshName + "12");
    MeshAsset* innerGeoPiece3 = gMeshAssetManager.GetMesh(meshName + "13"); // inner
    MeshAsset* geoEntrance = gMeshAssetManager.GetMesh(meshName + "14"); // entrance
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

        innerGeoPiece1, // !SideA && !Corner && !SideB
        innerGeoPiece0, // SideB
        innerGeoPiece1, // Corner
        innerGeoPiece0, // SideB && Corner
        innerGeoPiece0, // SideA
        innerGeoPiece2, // SideA && SideB
        innerGeoPiece0, // SideA && Corner
        innerGeoPiece2, // SideA && Corner && SideB
    };

    // rotation shortcuts
    const glm::mat3* Neg90 = &g_TileRotations[0];
    const glm::mat3* Pos90 = &g_TileRotations[1];
    const glm::mat3* NegPi = &g_TileRotations[2];
    const glm::mat3* Nullp = nullptr;

    const glm::mat3* pieceRotations[][8] = 
    {
        {Pos90, Pos90, Pos90, Pos90, Nullp, Pos90, Nullp, Nullp},
        {Nullp, Nullp, Nullp, Nullp, Neg90, Nullp, Neg90, Neg90},
        {Neg90, Neg90, Neg90, Neg90, NegPi, Neg90, NegPi, Nullp},
        {NegPi, NegPi, NegPi, NegPi, Pos90, NegPi, Pos90, Pos90},
    };

    const glm::mat3* innerPieceRotations[][8] = 
    {
        {Nullp, NegPi, Neg90, NegPi, Neg90, Pos90, Neg90, Pos90},
        {Nullp, Pos90, NegPi, Pos90, NegPi, Nullp, NegPi, Nullp},
        {Nullp, Nullp, Pos90, Nullp, Pos90, Neg90, Pos90, Neg90},
        {Nullp, Neg90, Nullp, Neg90, Nullp, NegPi, Nullp, NegPi}
    };

    for (MapTile* targetTile : floorTiles)
    {
        // entrance
        if (targetTile->mIsRoomEntrance)
        {
            mBaseTileConstructor.ExtendTileFloor(targetTile, geoEntrance, nullptr, &g_SubTileTranslations[0]);
            continue;
        }

        // inner tile
        if (targetTile->mIsRoomInnerTile)
        {
            mBaseTileConstructor.ExtendTileFloor(targetTile, innerGeoPiece3, nullptr,             &g_SubTileTranslations[0]);
            mBaseTileConstructor.ExtendTileFloor(targetTile, innerGeoPiece3, &g_TileRotations[0], &g_SubTileTranslations[1]);
            mBaseTileConstructor.ExtendTileFloor(targetTile, innerGeoPiece3, &g_TileRotations[2], &g_SubTileTranslations[2]);
            mBaseTileConstructor.ExtendTileFloor(targetTile, innerGeoPiece3, &g_TileRotations[1], &g_SubTileTranslations[3]);

            continue;
        }

        MapTile* neighbours[eDirection_COUNT];
        for (eDirection direction: gDirectionsCW)
        {
            neighbours[direction] = targetTile->SameNeighbourRoomInstance(direction) ? targetTile->mNeighbours[direction] : nullptr;
        }

        #define IS_INNER(direction) \
            (neighbours[direction] && neighbours[direction]->mIsRoomInnerTile)

        int subTiles[] = {
            (neighbours[eDirection_W] ? 0x04 : 0) | (neighbours[eDirection_NW] ? 0x02 : 0) | (neighbours[eDirection_N] ? 0x01 : 0), //SubtTopLeft
            (neighbours[eDirection_N] ? 0x04 : 0) | (neighbours[eDirection_NE] ? 0x02 : 0) | (neighbours[eDirection_E] ? 0x01 : 0), //SubtTopRight
            (neighbours[eDirection_E] ? 0x04 : 0) | (neighbours[eDirection_SE] ? 0x02 : 0) | (neighbours[eDirection_S] ? 0x01 : 0), //SubtBottomRight
            (neighbours[eDirection_S] ? 0x04 : 0) | (neighbours[eDirection_SW] ? 0x02 : 0) | (neighbours[eDirection_W] ? 0x01 : 0), //SubtBottomLeft
        };

        int exSubTiles[] = {
            (IS_INNER(eDirection_W) ? 0x04 : 0) | (IS_INNER(eDirection_NW) ? 0x02 : 0) | (IS_INNER(eDirection_N) ? 0x01 : 0), //SubtTopLeft
            (IS_INNER(eDirection_N) ? 0x04 : 0) | (IS_INNER(eDirection_NE) ? 0x02 : 0) | (IS_INNER(eDirection_E) ? 0x01 : 0), //SubtTopRight
            (IS_INNER(eDirection_E) ? 0x04 : 0) | (IS_INNER(eDirection_SE) ? 0x02 : 0) | (IS_INNER(eDirection_S) ? 0x01 : 0), //SubtBottomRight
            (IS_INNER(eDirection_S) ? 0x04 : 0) | (IS_INNER(eDirection_SW) ? 0x02 : 0) | (IS_INNER(eDirection_W) ? 0x01 : 0), //SubtBottomLeft
        };

        for (int isubtile = 0; isubtile < 4; ++isubtile)
        {
            if (exSubTiles[isubtile] > 0)
            {
                mBaseTileConstructor.ExtendTileFloor(targetTile, pieces[exSubTiles[isubtile] + 8], 
                    innerPieceRotations[isubtile][exSubTiles[isubtile]], &g_SubTileTranslations[isubtile]);
            }
            else
            {
                mBaseTileConstructor.ExtendTileFloor(targetTile, pieces[subTiles[isubtile]], 
                    pieceRotations[isubtile][subTiles[isubtile]], &g_SubTileTranslations[isubtile]);
            }
        }

        #undef IS_INNER
    }
}
