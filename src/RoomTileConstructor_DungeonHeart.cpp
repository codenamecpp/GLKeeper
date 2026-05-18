#include "stdafx.h"
#include "RoomTileConstructor_DungeonHeart.h"
#include "MeshAssetManager.h"
#include "Room.h"
#include "TileConstructor.h"

RoomTileConstructor_DungeonHeart::RoomTileConstructor_DungeonHeart(TileConstructor& baseConstructor)
    : RoomTileConstructor_5_by_5(baseConstructor)
{
}

void RoomTileConstructor_DungeonHeart::ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles)
{
    RoomTileConstructor_5_by_5::ConstructRoomFloor(roomInstance, floorTiles);

    const MapArea2D& locationArea = roomInstance->GetLocationArea();

    // construct inner geometry
    for (MapTile* mapTile: floorTiles)
    {
        const int ioffsetx = mapTile->mTileLocation.x - locationArea.x;
        const int ioffsety = mapTile->mTileLocation.y - locationArea.y;
        const int ioffset = (ioffsety * 5) + ioffsetx;

        if (ioffsetx == 2 && ioffsety == 2)
        {
            MeshAsset* stepsGeometry = gMeshAssetManager.GetMesh("DHeart BigSteps");
            MeshAsset* archesGeometry = gMeshAssetManager.GetMesh("DHeart Arches");

            // append steps and arches geometry
            mBaseTileConstructor.ExtendTileFloor(mapTile, archesGeometry);
            mBaseTileConstructor.ExtendTileFloor(mapTile, stepsGeometry);
            mBaseTileConstructor.ExtendTileFloor(mapTile, stepsGeometry, &g_TileRotations[3]);
            mBaseTileConstructor.ExtendTileFloor(mapTile, stepsGeometry, &g_TileRotations[4]);
            break;
        }
    } // for tile 
}
