#include "stdafx.h"
#include "RoomTileConstructor.h"
#include "RoomWallSection.h"
#include "Room.h"
#include "MeshAssetManager.h"
#include "TileConstructor.h"

RoomTileConstructor::RoomTileConstructor(TileConstructor& baseConstructor)
    : mBaseTileConstructor(baseConstructor)
{
}

RoomTileConstructor::~RoomTileConstructor()
{
}

void RoomTileConstructor::ConstructRoomFloor(Room* roomInstance, cxx::span<MapTile*> floorTiles)
{
    cxx_assert(roomInstance);

    // todo:
    // not implemented
}

void RoomTileConstructor::ConstructRoomWalls(Room* roomInstance, cxx::span<MapTile*> wallTiles, eTileFace wallFace)
{
    cxx_assert(roomInstance);

    RoomDefinition* definition = roomInstance->GetDefinition();
    if (!definition->mHasWalls)
        return;
  
    static const glm::vec3 sTranslationN {  0.0f,            0.0f,  -MAP_TILE_SIZE  };
    static const glm::vec3 sTranslationE {  MAP_TILE_SIZE,   0.0f,   0.0f          };
    static const glm::vec3 sTranslationS {  0.0f,            0.0f,   MAP_TILE_SIZE };
    static const glm::vec3 sTranslationW { -MAP_TILE_SIZE,   0.0f,   0.0f          };
  
    // get required geometry
    MeshAsset* wall0 = gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "4");
    MeshAsset* wall1 = gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "5");
    MeshAsset* wall2 = gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "6");
    MeshAsset* wall3 = gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "7");
    MeshAsset* wall4 = gMeshAssetManager.GetMesh(definition->mCompleteResource.mResourceName + "8");
  
    // traverse room wall sections
    glm::vec3 translation {};
    for (const RoomWallSection* wallSection : roomInstance->GetWallSections())
    {
        if (wallSection->GetFace() != wallFace)
            continue;

        const glm::mat3* rotation = nullptr;
        switch (wallSection->GetDirection())
        {
            case eDirection_N : 
                translation = sTranslationN;
                rotation = &g_TileRotations[2];
            break;
            case eDirection_E : 
                translation = sTranslationE;
                rotation = &g_TileRotations[1];
            break;
            case eDirection_S : 
                translation = sTranslationS;
            break;
            case eDirection_W : 
                translation = sTranslationW;
                rotation = &g_TileRotations[0];
            break;
        }

        for (MapTile* wallTile: wallTiles)
        {
            if (!wallSection->ContainsTile(wallTile)) 
                continue;

            TileFaceData& faceData = wallTile->mFaces[wallFace];
            cxx_assert(faceData.mWallExtendsRoom);

            // wall edge
            bool isEdgeL = wallSection->IsHead(wallTile);
            bool isEdgeR = wallSection->IsTail(wallTile);

            if (isEdgeL || isEdgeR)
            {
                // half-pieces have tiny negative offset
                const float translationOffset = (MAP_TILE_HALF_SIZE * 0.5f);

                glm::vec3 translationL = translation - (translation * translationOffset); 
                glm::vec3 translationR = translation - (translation * translationOffset);

                bool isHorz = wallSection->IsHorz();
                bool isInverted = (wallSection->GetDirection() == eDirection_S) || (wallSection->GetDirection() == eDirection_W);
                if (isHorz)
                {
                    translationL.x += translationOffset * (isInverted ?  1.0f : -1.0f);
                    translationR.x += translationOffset * (isInverted ? -1.0f :  1.0f);
                }
                else
                {
                    translationL.z += translationOffset * (isInverted ?  1.0f : -1.0f);
                    translationR.z += translationOffset * (isInverted ? -1.0f :  1.0f);
                }

                MeshAsset* pieceL = isInverted ? (isEdgeR ? wall1 : wall2) : (isEdgeL ? wall1 : wall2);
                MeshAsset* pieceR = isInverted ? (isEdgeL ? wall0 : wall2) : (isEdgeR ? wall0 : wall2);
                mBaseTileConstructor.ExtendTileWall(wallTile, wallSection->GetDirection(), pieceL, rotation, &translationL);
                mBaseTileConstructor.ExtendTileWall(wallTile, wallSection->GetDirection(), pieceR, rotation, &translationR);
            }
            else // inner wall
            {   
                MeshAsset* innerPiece = wallSection->IsOdd(wallTile) ? wall3 : wall4; // :)
                mBaseTileConstructor.ExtendTileWall(wallTile, wallSection->GetDirection(), innerPiece, rotation, &translation);
            }
        } // for
    } // for

    int bp = 0;
}
