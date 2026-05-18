#include "stdafx.h"
#include "RoomTileConstructor_HeroGate_3_by_1.h"
#include "MeshAssetManager.h"
#include "Room.h"
#include "TileConstructor.h"

RoomTileConstructor_HeroGate_3_by_1::RoomTileConstructor_HeroGate_3_by_1(TileConstructor& baseConstructor)
    : RoomTileConstructor(baseConstructor)
{
}

void RoomTileConstructor_HeroGate_3_by_1::ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles)
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
        gMeshAssetManager.GetMesh(completeResourceName + "3"),
        gMeshAssetManager.GetMesh(completeResourceName + "4"),
        gMeshAssetManager.GetMesh(completeResourceName + "5"),
    };
    // rotation shortcuts
    const glm::mat3* Neg90 = &g_TileRotations[0];
    const glm::mat3* Pos90 = &g_TileRotations[1];
    const glm::mat3* NegPi = &g_TileRotations[2];
    const glm::mat3* Nullp = nullptr;

    bool invertSides = false;
    const glm::mat3* pieceRotation = Nullp;
    switch (roomInstance->GetRoomDirection())
    {
        case eDirection_N: 
            invertSides = true;
            pieceRotation = Neg90; 
        break;
        case eDirection_E: 
            invertSides = true;
            pieceRotation = NegPi; 
        break;
        case eDirection_S: pieceRotation = Pos90; break;
        case eDirection_W: break; // default pieces rotation - West
    }

    const MapArea2D& locationArea = roomInstance->GetLocationArea();
    for (MapTile* targetTile : floorTiles)
    {
        const int ioffsetx = targetTile->mTileLocation.x - locationArea.x;
        const int ioffsety = targetTile->mTileLocation.y - locationArea.y;
        const int ioffset = std::max(ioffsetx, ioffsety) % 3;

        MeshAsset* piece = pieces[(invertSides ? (3 - ioffset - 1) : ioffset)];
        mBaseTileConstructor.ExtendTileFloor(targetTile, piece, pieceRotation);
    }
}

void RoomTileConstructor_HeroGate_3_by_1::ConstructRoomWalls(Room* roomInstance, cxx::span<MapTile*> wallTiles, eTileFace wallFace)
{
    cxx_assert(roomInstance);

    RoomDefinition* definition = roomInstance->GetDefinition();
    if (!definition->mHasWalls)
        return;

    bool isDestroyed = false;

    std::string completeResourceName = definition->mCompleteResource.mResourceName;
    MeshAsset* pieces[] = 
    {
        gMeshAssetManager.GetMesh(completeResourceName + "0"), // wall
        gMeshAssetManager.GetMesh(completeResourceName + "1"), // wall, gate
        gMeshAssetManager.GetMesh(completeResourceName + "2"), // wall
        gMeshAssetManager.GetMesh(completeResourceName + "6"), // wall, solid
    };

    if (isDestroyed)
    {
        completeResourceName.append("destroyed");
    }

    static const glm::vec3 sTranslationN {  0.0f,            0.0f,  -MAP_TILE_SIZE  };
    static const glm::vec3 sTranslationE {  MAP_TILE_SIZE,   0.0f,   0.0f          };
    static const glm::vec3 sTranslationS {  0.0f,            0.0f,   MAP_TILE_SIZE };
    static const glm::vec3 sTranslationW { -MAP_TILE_SIZE,   0.0f,   0.0f          };

    // rotation shortcuts
    const glm::mat3* Neg90 = &g_TileRotations[0];
    const glm::mat3* Pos90 = &g_TileRotations[1];
    const glm::mat3* NegPi = &g_TileRotations[2];

    for (const RoomWallSection* wallSection : roomInstance->GetWallSections())
    {
        if (wallSection->GetFace() != wallFace)
            continue;

        bool invertSides = false;

        const glm::mat3* pieceRotation = nullptr;
        const glm::vec3* pieceTranslation = nullptr;
        switch (wallSection->GetDirection())
        {
            case eDirection_N: 
                pieceTranslation = &sTranslationN;
                pieceRotation = Neg90; 
                invertSides = true;
            break;
            case eDirection_E: 
                pieceTranslation = &sTranslationE;
                pieceRotation = NegPi; 
                invertSides = true;
            break;
            case eDirection_S: 
                pieceTranslation = &sTranslationS;
                pieceRotation = Pos90; 
            break;
            case eDirection_W: 
                pieceTranslation = &sTranslationW;
            break; // default pieces rotation - West
        }

        bool sameDirectionAsRoom = wallSection->GetDirection() == roomInstance->GetRoomDirection();

        for (MapTile* wallTile: wallTiles)
        {
            if (!wallSection->ContainsTile(wallTile)) continue;

            MeshAsset* wallPiece = (sameDirectionAsRoom && wallSection->IsInner(wallTile)) ? pieces[1] : pieces[3];
            if (sameDirectionAsRoom)
            {
                if (wallSection->IsHead(wallTile) || wallSection->IsTail(wallTile))
                {
                    wallPiece = (invertSides ? wallSection->IsTail(wallTile) : wallSection->IsHead(wallTile)) ? 
                        pieces[0] : 
                        pieces[2];
                }
            }
            mBaseTileConstructor.ExtendTileMesh(wallTile, wallSection->GetFace(), wallPiece, pieceRotation, pieceTranslation);
        }
    }
    int bp = 0;
}
