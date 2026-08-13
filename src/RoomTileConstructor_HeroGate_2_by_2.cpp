#include "stdafx.h"
#include "RoomTileConstructor_HeroGate_2_by_2.h"
#include "MeshAssetManager.h"
#include "Room.h"
#include "TileConstructor.h"

RoomTileConstructor_HeroGate_2_by_2::RoomTileConstructor_HeroGate_2_by_2(TileConstructor& baseConstructor)
    : RoomTileConstructor(baseConstructor)
{
}

void RoomTileConstructor_HeroGate_2_by_2::ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles)
{
    cxx_assert(roomInstance);

    bool isDestroyed = false;

    RoomDefinition* definition = roomInstance->GetDefinition();

    std::string completeResourceName = definition->mCompleteResource.mResourceName;
    if (isDestroyed)
    {
        completeResourceName.append("destroyed");
    }

    MeshAsset* pieces[] = 
    {
        gMeshAssetManager.GetMesh(completeResourceName + "0"),
        gMeshAssetManager.GetMesh(completeResourceName + "1"),
        gMeshAssetManager.GetMesh(completeResourceName + "2"),
        gMeshAssetManager.GetMesh(completeResourceName + "3"),
    };
    const MapArea2D& locationArea = roomInstance->GetLocationArea();
    for (MapTile* targetTile : floorTiles)
    {
        const int ioffsetx = targetTile->mLocation.x - locationArea.x;
        const int ioffsety = targetTile->mLocation.y - locationArea.y;
        const int ioffset = (ioffsety * 2) + ioffsetx;

        cxx_assert(ioffset >= 0 && ioffset <= 4);
        if (ioffset < 0 || ioffset > 4)
            return;

        mBaseTileConstructor.ExtendTileFloor(targetTile, pieces[ioffset]);
    }
}
