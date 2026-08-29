#include "stdafx.h"
#include "RoomTileConstructor_HeroGateFrontend.h"
#include "MeshAssetManager.h"
#include "Room.h"
#include "TileConstructor.h"

RoomTileConstructor_HeroGateFrontend::RoomTileConstructor_HeroGateFrontend(TileConstructor& baseConstructor)
    : RoomTileConstructor(baseConstructor)
{
}

void RoomTileConstructor_HeroGateFrontend::ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles)
{
    cxx_assert(roomInstance);

    const Rect2D& locationArea = roomInstance->GetLocationArea();

    RoomDefinition* definition = roomInstance->GetDefinition();

    for (MapTile* targetTile: floorTiles)
    {
        int tilex = targetTile->mLocation.x - locationArea.x;
        int tiley = targetTile->mLocation.y - locationArea.y;
        if (tiley > 4)
        {
            tilex = 0;
        }
        int imesh = tiley * 3 + tilex;

        MeshAsset* meshResource = gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + std::to_string(imesh + 1));
        mBaseTileConstructor.ExtendTileFloor(targetTile, meshResource);
    }
}

void RoomTileConstructor_HeroGateFrontend::ConstructRoomWalls(Room* roomInstance, cxx::span<MapTile*> wallTiles, eTileFace wallFace)
{
    // do nothing
}
