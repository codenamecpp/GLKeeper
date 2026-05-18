#include "stdafx.h"
#include "RoomTileConstructor_3_by_3.h"
#include "MeshAssetManager.h"
#include "Room.h"
#include "TileConstructor.h"

RoomTileConstructor_3_by_3::RoomTileConstructor_3_by_3(TileConstructor& baseConstructor)
    : RoomTileConstructor(baseConstructor)
{
}

void RoomTileConstructor_3_by_3::ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles)
{
    cxx_assert(roomInstance);

    const MapArea2D& locationArea = roomInstance->GetLocationArea();

    RoomDefinition* definition = roomInstance->GetDefinition();
    MeshAsset* pieces[] = 
    {
        gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "0"),
        gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "1"),
        gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "2"),
        gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "3"),
        gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "4"),
        gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "5"),
        gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "6"),
        gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "7"),
        gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "8"),
    };

    for (MapTile* targetTile : floorTiles)
    {
        const int ioffsetx = targetTile->mTileLocation.x - locationArea.x;
        const int ioffsety = targetTile->mTileLocation.y - locationArea.y;
        const int ioffset = (ioffsety * 3) + ioffsetx;

        cxx_assert(ioffset >= 0 && ioffset <= 8);
        if (ioffset < 0 || ioffset > 8)
            return;

        mBaseTileConstructor.ExtendTileFloor(targetTile, pieces[ioffset]);
    }
}
