#include "stdafx.h"
#include "RoomTileConstructor_5_by_5.h"
#include "MeshAssetManager.h"
#include "Room.h"
#include "TileConstructor.h"

RoomTileConstructor_5_by_5::RoomTileConstructor_5_by_5(TileConstructor& baseConstructor)
    : RoomTileConstructor(baseConstructor)
{
}

void RoomTileConstructor_5_by_5::ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles)
{
    cxx_assert(roomInstance);

    const MapArea2D& locationArea = roomInstance->GetLocationArea();

    RoomDefinition* definition = roomInstance->GetDefinition();

    // process each tile
    for (MapTile* mapTile : floorTiles)
    {
        const glm::mat3* rotation = nullptr;

        // get tile offset
        const int ioffsetx = mapTile->mTileLocation.x - locationArea.x;
        const int ioffsety = mapTile->mTileLocation.y - locationArea.y;
        const int ioffset = (ioffsety * 5) + ioffsetx;

        // corner
        bool isCorner = 
            ((ioffsetx == 0) || (ioffsetx == 4)) &&
            ((ioffsety == 0) || (ioffsety == 4));

        if (isCorner)
        {
            MeshAsset* geometry = gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "3");
            switch (ioffset)
            {
                case 0 : rotation = &g_TileRotations[1]; // top left
                    break; 
                case 4 : // top right
                    break;
                case 20 : rotation = &g_TileRotations[2]; // bottom left
                    break;
                case 24 : rotation = &g_TileRotations[0]; // bottom right
                    break;
            }

            mBaseTileConstructor.ExtendTileFloor(mapTile, geometry, rotation);
            continue;
        }

        // Outer layer sides
        bool isOuterLayerSides =
            (ioffsetx == 0) || (ioffsetx == 4) ||
            (ioffsety == 0) || (ioffsety == 4);

        if (isOuterLayerSides)
        {
            MeshAsset* geometry = gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "2");
            if (ioffsetx == 0 || ioffsetx == 4)
            {
                rotation = (ioffsetx == 0) ? &g_TileRotations[2] : nullptr;
            }
            else
            {
                rotation = (ioffsety == 0) ? &g_TileRotations[1] : &g_TileRotations[0];
            }

            mBaseTileConstructor.ExtendTileFloor(mapTile, geometry, rotation);
            continue;
        }

        // The inner ring, corners
        bool isInnerCorner =
            ((ioffsetx == 1) || (ioffsetx == 3)) &&
            ((ioffsety == 1) || (ioffsety == 3));

        if (isInnerCorner)
        {
            MeshAsset* geometry = gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "0");
            switch (ioffset)
            {
                case 6 : // top left
                    break; 
                case 8 : rotation = &g_TileRotations[0];// top right
                    break;
                case 16 : rotation = &g_TileRotations[1]; // bottom left
                    break;
                case 18 : rotation = &g_TileRotations[2]; // bottom right
                    break;
            }

            mBaseTileConstructor.ExtendTileFloor(mapTile, geometry, rotation);
            continue;
        }

        // The inner ring, sides
        bool isInnerSides =
            (ioffsetx == 1) || (ioffsetx == 3) ||
            (ioffsety == 1) || (ioffsety == 3);

        if (isInnerSides)
        {
            MeshAsset* geometry = gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "1");
            if (ioffsetx == 1 || ioffsetx == 3)
            {
                rotation = (ioffsetx == 1) ? &g_TileRotations[1] : &g_TileRotations[0];
            }
            else
            {
                rotation = (ioffsety == 1) ? nullptr : &g_TileRotations[2];
            }

            mBaseTileConstructor.ExtendTileFloor(mapTile, geometry, rotation);
            continue;
        }
    }
}
