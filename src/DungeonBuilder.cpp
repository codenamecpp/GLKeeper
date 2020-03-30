#include "pch.h"
#include "DungeonBuilder.h"
#include "MapTile.h"
#include "ModelAsset.h"
#include "TexturesManager.h"
#include "ModelAssetsManager.h"
#include "GameWorld.h"

void DungeonBuilder::ExtendTileMesh(MapTile* mapTile, eTileFace faceid, ModelAsset* asset, const glm::mat3* rot, const glm::vec3* trans)
{
    debug_assert(mapTile);
    debug_assert(asset);
    debug_assert(faceid < eTileFace_COUNT);

    // destination geometry
    TileFaceData& tileFace = mapTile->mFaces[faceid];
    const glm::vec3 tileTranslation = 
    {
        mapTile->mTileLocation.x + (trans ? trans->x : 0.0f), trans ? trans->y : 0.0f,
        mapTile->mTileLocation.y + (trans ? trans->z : 0.0f)
    };

    TerrainDefinition* terrainDefinition = mapTile->GetTerrain();

    // append submeshes
    for (const ModelAsset::SubMesh& pieceSprite: asset->mMeshArray)
    {
        // choose material
        const ModelAsset::SubMeshMaterial& sourceMaterial = asset->mMaterialsArray[pieceSprite.mMaterialIndex];

        int textureIndex = 0;
        if (terrainDefinition->mTextureFrames > 1)
        {
            const int NumVariations = sourceMaterial.mTextures.size();
            if (NumVariations > 1)
            {
                switch (faceid)
                {
                    case eTileFace_SideN:
                    case eTileFace_SideE:
                    case eTileFace_SideS:
                    case eTileFace_SideW:
                    {
                        if (terrainDefinition->mIsDecay)
                        {
                            if (tileFace.mWallSection == nullptr) // wall not specified
                            {
                                textureIndex = NumVariations - 1;
                            }
                            break;
                        }
                    }
                    break;

                    case eTileFace_Floor:
                    case eTileFace_Ceiling:
                    {
                        if (terrainDefinition->mIsDecay)
                            break;

                        if (terrainDefinition->mHasRandomTexture)
                        {
                            // select random texture including original
                            textureIndex = mapTile->mRandomValue % NumVariations;
                            break;
                        }
                    }
                    break;
                }
            }  // numvariations
        }

        if (pieceSprite.mLODsArray.empty() ||  // sometimes triangles is missed ?
            pieceSprite.mLODsArray[0].mTriangleArray.empty())
        {
            continue;
        }

        // add terrain mesh piece
        tileFace.mMeshArray.emplace_back();

        TileMesh& destMeshGroup = tileFace.mMeshArray.back();
        destMeshGroup.mMaterial.mDiffuseTexture = gTexturesManager.GetTexture2D(sourceMaterial.mTextures[textureIndex]);
        if (sourceMaterial.mEnvMappingTexture.length())
        {
            destMeshGroup.mMaterial.mEnvMappingTexture = gTexturesManager.GetTexture2D(sourceMaterial.mEnvMappingTexture);
        }
        // force default render states for terrain mesh with disabled alphablending
        destMeshGroup.mMaterial.mRenderStates = RenderStates();     

        // source data
        const glm::vec3* sourcePositions = &pieceSprite.mVertexPositionArray[0];
        const glm::vec3* sourceNormals = &pieceSprite.mVertexNormalArray[0];
        const glm::vec2* sourceTexcoord = &pieceSprite.mVertexTexCoordArray[0];
        // transfrom vertex position
        destMeshGroup.mVertices.resize(pieceSprite.mFrameVerticesCount);
        if (rot)
        {
            unsigned int ivertex = 0;
            for (Vertex3D_Terrain& vertex : destMeshGroup.mVertices)
            {
                const glm::vec3 transformed = (*rot * glm::vec3{
                    sourcePositions[ivertex].x, 
                    sourcePositions[ivertex].y, 
                    sourcePositions[ivertex].z});

                const glm::vec3 transformedNormal = (*rot * glm::vec3{
                    sourceNormals[ivertex].x, 
                    sourceNormals[ivertex].y, 
                    sourceNormals[ivertex].z});

                vertex.mTileX = mapTile->mTileLocation.x;
                vertex.mTileY = mapTile->mTileLocation.y;
                vertex.mPosition = transformed + tileTranslation;
                vertex.mNormal = transformedNormal;
                vertex.mTexcoord = sourceTexcoord[ivertex];
                ++ivertex;
            } // for ivertex
        }
        else // if rotation
        {
            unsigned int ivertex = 0;
            for (Vertex3D_Terrain& vertex : destMeshGroup.mVertices)
            {
                vertex.mTileX = mapTile->mTileLocation.x;
                vertex.mTileY = mapTile->mTileLocation.y;
                vertex.mPosition = tileTranslation + sourcePositions[ivertex];
                vertex.mNormal = sourceNormals[ivertex];
                vertex.mTexcoord = sourceTexcoord[ivertex];
                ++ivertex;
            } // for ivertex
        } // if rotation

        // copy triangles unmodified
        destMeshGroup.mTriangles = pieceSprite.mLODsArray[0].mTriangleArray;
    }
}

void DungeonBuilder::BuildTerrainMesh(MapTile* mapTile)
{
    debug_assert(mapTile);
    mapTile->ClearTileMesh();

    ConstructTerrainFloor(mapTile);

    TerrainDefinition* terrainDef = mapTile->GetTerrain();
    if (terrainDef->mIsSolid)
    {
        ConstructTerrainWalls(mapTile);
    }

    // todo: ceiling
}

void DungeonBuilder::BuildTerrainMesh(MapTile* mapTile, eTileFace faceid)
{
    mapTile->ClearTileMesh(faceid);

    if (faceid == eTileFace_Floor)
    {
        ConstructTerrainFloor(mapTile);
    }

    // construct wall
    if (faceid == eTileFace_SideE || faceid == eTileFace_SideN || faceid == eTileFace_SideS || faceid == eTileFace_SideW)
    {
        TerrainDefinition* terrainDef = mapTile->GetTerrain();
        if (terrainDef->mIsSolid)
        {
            ConstructTerrainWall(mapTile, faceid);
        }
    }

    // todo: ceiling
}

void DungeonBuilder::ConstructTerrainWalls(MapTile* mapTile)
{
    for (eTileFace faceid: {eTileFace_SideN, eTileFace_SideE, eTileFace_SideS, eTileFace_SideW})
    {
        ConstructTerrainWall(mapTile, faceid);
    }
}

void DungeonBuilder::ConstructTerrainWall(MapTile* mapTile, eTileFace faceid)
{
    const glm::mat3* rotation = nullptr;
    switch (faceid)
    {
        case eTileFace_SideN: 
            rotation = nullptr;
        break;
        case eTileFace_SideS: 
            rotation = &g_TileRotations[2];
        break;
        case eTileFace_SideE: 
            rotation = &g_TileRotations[0];
        break;
        case eTileFace_SideW: 
            rotation = &g_TileRotations[1];
        break;
        default:
            debug_assert(false);
        return;
    }

    TerrainDefinition* terrain = mapTile->GetTerrain();
    debug_assert(terrain->mIsSolid);
    // test side wall is required
    if (ShouldBuildSideWall(mapTile, faceid))
    {
        ModelAsset* asset = gModelsManager.LoadModelAsset(terrain->mResourceSide.mResourceName);
        debug_assert(asset);
        ExtendTileMesh(mapTile, faceid, asset, rotation);
    }
}

void DungeonBuilder::ConstructTerrainFloor(MapTile* mapTile)
{
    TerrainDefinition* terrainDef = mapTile->GetTerrain();

    // if terrain is bridge we building only water bed
    if (gGameWorld.IsRoomTypeTerrain(terrainDef))
    {
        TerrainDefinition* baseTerrainDef = mapTile->GetBaseTerrain();
        if (baseTerrainDef->mIsWater || baseTerrainDef->mIsLava)
        {
            ConstructTerrainWaterBed(mapTile, mapTile->GetBaseTerrain()->GetCellResource());
        }
        // dont build room floor geometry here
        return;
    }

    ArtResource* cellResource = terrainDef->GetCellResource();
    if (!cellResource || !cellResource->IsDefined())
        return;

    // water bed
    if (terrainDef->mConstructionTypeWater)
    {
        ConstructTerrainWaterBed(mapTile, cellResource);
    }
    // construct terrain quad
    else if (terrainDef->mConstructionTypeQuad)
    {
        ConstructTerrainQuad(mapTile, cellResource);
    }
    // construct terrain normal
    else
    {
        ModelAsset* asset = gModelsManager.LoadModelAsset(cellResource->mResourceName);
        debug_assert(asset);
        ExtendTileMesh(mapTile, eTileFace_Floor, asset);
    }
}

void DungeonBuilder::ConstructTerrainQuad(MapTile* mapTile, ArtResource* artResource)
{
    std::string meshName = artResource->mResourceName;

    // coloured to player color
    TerrainDefinition* tileTerrainDef = mapTile->GetTerrain();
    if (tileTerrainDef->mPlayerColouredPath || tileTerrainDef->mPlayerColouredWall) 
    {
        const char* suffix = cxx::va("%d_", (mapTile->mOwnerId == ePlayerID_Null) ? 0 : mapTile->mOwnerId - 1);
        meshName.append(suffix);
    }

    int subTiles[4];
    if (tileTerrainDef->mIsSolid)
    {
        //SubtTopLeft
        subTiles[0] = 
            (NeighbourTileSolid(mapTile, eDirection_W) ? 0x04 : 0) | 
            (NeighbourTileSolid(mapTile, eDirection_NW) ? 0x02 : 0) | 
            (NeighbourTileSolid(mapTile, eDirection_N) ? 0x01 : 0);
        //SubtTopRight
        subTiles[1] = 
            (NeighbourTileSolid(mapTile, eDirection_N) ? 0x04 : 0) | 
            (NeighbourTileSolid(mapTile, eDirection_NE) ? 0x02 : 0) | 
            (NeighbourTileSolid(mapTile, eDirection_E) ? 0x01 : 0);
        //SubtBottomRight
        subTiles[2] = 
            (NeighbourTileSolid(mapTile, eDirection_E) ? 0x04 : 0) | 
            (NeighbourTileSolid(mapTile, eDirection_SE) ? 0x02 : 0) | 
            (NeighbourTileSolid(mapTile, eDirection_S) ? 0x01 : 0);
        //SubtBottomLeft
        subTiles[3] = 
            (NeighbourTileSolid(mapTile, eDirection_S) ? 0x04 : 0) | 
            (NeighbourTileSolid(mapTile, eDirection_SW) ? 0x02 : 0) | 
            (NeighbourTileSolid(mapTile, eDirection_W) ? 0x01 : 0);
    }
    else
    {
        //SubtTopLeft
        subTiles[0] = 
            (NeighbourHasSameTerrain(mapTile, eDirection_W) ? 0x04 : 0) | 
            (NeighbourHasSameTerrain(mapTile, eDirection_NW) ? 0x02 : 0) | 
            (NeighbourHasSameTerrain(mapTile, eDirection_N) ? 0x01 : 0);
        //SubtTopRight
        subTiles[1] = 
            (NeighbourHasSameTerrain(mapTile, eDirection_N) ? 0x04 : 0) | 
            (NeighbourHasSameTerrain(mapTile, eDirection_NE) ? 0x02 : 0) | 
            (NeighbourHasSameTerrain(mapTile, eDirection_E) ? 0x01 : 0);
        //SubtBottomRight
        subTiles[2] = 
            (NeighbourHasSameTerrain(mapTile, eDirection_E) ? 0x04 : 0) | 
            (NeighbourHasSameTerrain(mapTile, eDirection_SE) ? 0x02 : 0) | 
            (NeighbourHasSameTerrain(mapTile, eDirection_S) ? 0x01 : 0);
        //SubtBottomLeft
        subTiles[3] = 
            (NeighbourHasSameTerrain(mapTile, eDirection_S) ? 0x04 : 0) | 
            (NeighbourHasSameTerrain(mapTile, eDirection_SW) ? 0x02 : 0) | 
            (NeighbourHasSameTerrain(mapTile, eDirection_W) ? 0x01 : 0);
    }

    const glm::mat3* rotations[] =
    {
        &g_TileRotations[2],
        &g_TileRotations[1],
        nullptr,
        &g_TileRotations[0]
    };

    // pieces
    ModelAsset* piece0 = gModelsManager.LoadModelAsset(meshName + "0");
    ModelAsset* piece1 = gModelsManager.LoadModelAsset(meshName + "1");
    ModelAsset* piece2 = gModelsManager.LoadModelAsset(meshName + "2");
    ModelAsset* piece3 = gModelsManager.LoadModelAsset(meshName + "3");
    ModelAsset* piece4 = gModelsManager.LoadModelAsset(meshName + "4");
    ModelAsset* pieces[8] = 
    {
        piece1, piece4, piece1, piece4, // 1, 4, 0, 4
        piece0, piece2, piece0, piece3
    };
    
    for (int isubtile = 0; isubtile < 4; ++isubtile)
    {
        ExtendTileMesh(mapTile, eTileFace_Floor, pieces[subTiles[isubtile]], rotations[isubtile], &g_SubTileTranslations[isubtile]);
    }
}

void DungeonBuilder::ConstructTerrainWaterBed(MapTile* mapTile, ArtResource* artResource)
{
    const glm::mat3* rotations[] =
    {
        &g_TileRotations[0],
        &g_TileRotations[1],
        &g_TileRotations[2]
    };

    static const unsigned char sDirsBits[] =
    {
        0x01U, //DIRECTION_N
        0x10U, //DIRECTION_NE
        0x02U, //DIRECTION_E
        0x20U, //DIRECTION_SE
        0x04U, //DIRECTION_S
        0x40U, //DIRECTION_SW
        0x08U, //DIRECTION_W
        0x80U, //DIRECTION_NW
    };

    const unsigned char bits =
    // sides
        (!NeighbourHasSameBaseTerrain(mapTile, eDirection_N) ? sDirsBits[eDirection_N] : 0U) | // top
        (!NeighbourHasSameBaseTerrain(mapTile, eDirection_E) ? sDirsBits[eDirection_E] : 0U) | // right
        (!NeighbourHasSameBaseTerrain(mapTile, eDirection_S) ? sDirsBits[eDirection_S] : 0U) | // bottom
        (!NeighbourHasSameBaseTerrain(mapTile, eDirection_W) ? sDirsBits[eDirection_W] : 0U) | // left
    // corners
        (!NeighbourHasSameBaseTerrain(mapTile, eDirection_NE) ? sDirsBits[eDirection_NE] : 0U) | // top-right
        (!NeighbourHasSameBaseTerrain(mapTile, eDirection_SE) ? sDirsBits[eDirection_SE] : 0U) | // bottom-right
        (!NeighbourHasSameBaseTerrain(mapTile, eDirection_SW) ? sDirsBits[eDirection_SW] : 0U) | // bottom-left
        (!NeighbourHasSameBaseTerrain(mapTile, eDirection_NW) ? sDirsBits[eDirection_NW] : 0U); // top-left

    const unsigned char cornersBits = bits & 0xF0U;
    const unsigned char sidesBits = bits & 0x0FU;

    ModelAsset* piece0 = gModelsManager.LoadModelAsset(artResource->mResourceName + "0");
    ModelAsset* piece1 = gModelsManager.LoadModelAsset(artResource->mResourceName + "1");
    ModelAsset* piece2 = gModelsManager.LoadModelAsset(artResource->mResourceName + "2");
    ModelAsset* piece3 = gModelsManager.LoadModelAsset(artResource->mResourceName + "3");

    // simplest one
    if (bits == 0)
    {
        ExtendTileMesh(mapTile, eTileFace_Floor, piece3);
        return;
    }

    // corner
    bool isSingleCorner = 
        bits == sDirsBits[eDirection_NE] || bits == sDirsBits[eDirection_SE] ||
        bits == sDirsBits[eDirection_SW] || bits == sDirsBits[eDirection_NW];
    if (isSingleCorner)
    {
        static std::pair<unsigned char, const glm::mat3*> cornerDirections[] = {
            {sDirsBits[eDirection_NE], nullptr}, 
            {sDirsBits[eDirection_SE], rotations[0]}, 
            {sDirsBits[eDirection_SW], rotations[2]}, 
            {sDirsBits[eDirection_NW], rotations[1]}
        };
        for (const auto& pentry : cornerDirections)
        {
            if (bits == pentry.first)
            {
                ExtendTileMesh(mapTile, eTileFace_Floor, piece2, pentry.second);
                break;
            }
        }
        return;
    }

    // side
    static unsigned char sBitsSideN = sDirsBits[eDirection_N] | sDirsBits[eDirection_NW] | sDirsBits[eDirection_NE];
    static unsigned char sBitsSideE = sDirsBits[eDirection_NE] | sDirsBits[eDirection_E] | sDirsBits[eDirection_SE];
    static unsigned char sBitsSideS = sDirsBits[eDirection_SE] | sDirsBits[eDirection_S] | sDirsBits[eDirection_SW];
    static unsigned char sBitsSideW = sDirsBits[eDirection_NW] | sDirsBits[eDirection_W] | sDirsBits[eDirection_SW];

    bool isSingleSide =
        (sidesBits == sDirsBits[eDirection_N] && (bits & ~sBitsSideN) == 0) ||
        (sidesBits == sDirsBits[eDirection_E] && (bits & ~sBitsSideE) == 0) ||
        (sidesBits == sDirsBits[eDirection_S] && (bits & ~sBitsSideS) == 0) ||
        (sidesBits == sDirsBits[eDirection_W] && (bits & ~sBitsSideW) == 0);
    if (isSingleSide)
    {
        static std::pair<unsigned char, const glm::mat3*> directions[] = {
            {sDirsBits[eDirection_N], rotations[2]}, 
            {sDirsBits[eDirection_E], rotations[1]}, 
            {sDirsBits[eDirection_S], nullptr}, 
            {sDirsBits[eDirection_W], rotations[0]}
        };
        for (const auto& pentry : directions)
        {
            if (sidesBits == pentry.first)
            {
                ExtendTileMesh(mapTile, eTileFace_Floor, piece0, pentry.second);
                break;
            }
        }
        return;
    }

    // land corner
    static unsigned char sBitsLandCornerNE = sDirsBits[eDirection_N] | sDirsBits[eDirection_NE] | sDirsBits[eDirection_E];
    static unsigned char sBitsLandCornerSE = sDirsBits[eDirection_E] | sDirsBits[eDirection_SE] | sDirsBits[eDirection_S];
    static unsigned char sBitsLandCornerSW = sDirsBits[eDirection_S] | sDirsBits[eDirection_SW] | sDirsBits[eDirection_W];
    static unsigned char sBitsLandCornerNW = sDirsBits[eDirection_W] | sDirsBits[eDirection_NW] | sDirsBits[eDirection_N];

    bool isLandCorner =
        ((bits & sBitsLandCornerNE) == sBitsLandCornerNE) && ((bits & sBitsLandCornerSW) == 0) ||
        ((bits & sBitsLandCornerSE) == sBitsLandCornerSE) && ((bits & sBitsLandCornerNW) == 0) ||
        ((bits & sBitsLandCornerSW) == sBitsLandCornerSW) && ((bits & sBitsLandCornerNE) == 0) ||
        ((bits & sBitsLandCornerNW) == sBitsLandCornerNW) && ((bits & sBitsLandCornerSE) == 0);
    if (isLandCorner)
    {
        static std::pair<unsigned char, const glm::mat3*> directions[] = {
            {sDirsBits[eDirection_N] | sDirsBits[eDirection_E], rotations[2]}, 
            {sDirsBits[eDirection_E] | sDirsBits[eDirection_S], rotations[1]}, 
            {sDirsBits[eDirection_W] | sDirsBits[eDirection_S], nullptr}, 
            {sDirsBits[eDirection_N] | sDirsBits[eDirection_W], rotations[0]}
        };
        for (const auto& pentry : directions)
        {
            if (sidesBits == pentry.first)
            {
                ExtendTileMesh(mapTile, eTileFace_Floor, piece1, pentry.second);
                break;
            }
        }
        return;
    }

    int subTiles[] = {
    //SubtTopLeft
        (NeighbourHasSameBaseTerrain(mapTile, eDirection_W) ? 0x04 : 0) | 
        (NeighbourHasSameBaseTerrain(mapTile, eDirection_NW) ? 0x02 : 0) | 
        (NeighbourHasSameBaseTerrain(mapTile, eDirection_N) ? 0x01 : 0),
    //SubtTopRight
        (NeighbourHasSameBaseTerrain(mapTile, eDirection_N) ? 0x04 : 0) | 
        (NeighbourHasSameBaseTerrain(mapTile, eDirection_NE) ? 0x02 : 0) | 
        (NeighbourHasSameBaseTerrain(mapTile, eDirection_E) ? 0x01 : 0),
    //SubtBottomRight
        (NeighbourHasSameBaseTerrain(mapTile, eDirection_E) ? 0x04 : 0) | 
        (NeighbourHasSameBaseTerrain(mapTile, eDirection_SE) ? 0x02 : 0) | 
        (NeighbourHasSameBaseTerrain(mapTile, eDirection_S) ? 0x01 : 0),
    //SubtBottomLeft
        (NeighbourHasSameBaseTerrain(mapTile, eDirection_S) ? 0x04 : 0) | 
        (NeighbourHasSameBaseTerrain(mapTile, eDirection_SW) ? 0x02 : 0) | 
        (NeighbourHasSameBaseTerrain(mapTile, eDirection_W) ? 0x01 : 0)
    };

    ModelAsset* piece4 = gModelsManager.LoadModelAsset(artResource->mResourceName + "4");
    ModelAsset* piece5 = gModelsManager.LoadModelAsset(artResource->mResourceName + "5");
    ModelAsset* piece6 = gModelsManager.LoadModelAsset(artResource->mResourceName + "6");
    ModelAsset* piece7 = gModelsManager.LoadModelAsset(artResource->mResourceName + "7");
    ModelAsset* geoIndices[8] = {
        piece5, piece4, piece5, piece4, 
        piece4, piece6, piece4, piece7
    };

    static const glm::mat3* subtileRotations[][8] =
    {
        {rotations[0], rotations[0], rotations[0], rotations[0], rotations[2], rotations[1], rotations[2], nullptr},
        {rotations[2], rotations[2], rotations[2], rotations[2], rotations[1], nullptr, rotations[1], nullptr},
        {rotations[1], rotations[1], rotations[1], rotations[1], nullptr, rotations[0], nullptr, nullptr},
        {nullptr, nullptr, nullptr, nullptr, rotations[0], rotations[2], rotations[0], nullptr},
    };
    
    for (int isubtile = 0; isubtile < 4; ++isubtile)
    {
        ExtendTileMesh(mapTile, eTileFace_Floor, geoIndices[subTiles[isubtile]], 
            subtileRotations[isubtile][subTiles[isubtile]], &g_SubTileTranslations[isubtile]);
    }
}

bool DungeonBuilder::ShouldBuildSideWall(MapTile* mapTile, eTileFace faceid) const
{
    eDirection direction = FaceIdToDirection(faceid);
    if (MapTile* neighbour = mapTile->mNeighbours[direction])
    {
        TerrainDefinition* neighbourTerrainDef = neighbour->GetTerrain();
        if (neighbourTerrainDef->mIsSolid)
            return false;

        if (RoomHandlesWalls(neighbour))
        {
            TerrainDefinition* tileTerrainDef = mapTile->GetTerrain();
            if (tileTerrainDef->mAllowRoomWalls)
                return false;
        }
    }
    return true;
}

bool DungeonBuilder::NeighbourTileSolid(MapTile* mapTile, eDirection direction) const
{
    if (MapTile* neighbourTile = mapTile->mNeighbours[direction])
    {
        TerrainDefinition* neighbourTerrainDef = neighbourTile->GetTerrain();
        return neighbourTerrainDef->mIsSolid;
    }
    return false;
}

bool DungeonBuilder::NeighbourHasSameTerrain(MapTile* mapTile, eDirection direction) const
{
    if (MapTile* neighbourTile = mapTile->mNeighbours[direction])
    {
        return mapTile->GetTerrain() == neighbourTile->GetTerrain();
    }
    return false;
}

bool DungeonBuilder::NeighbourHasSameBaseTerrain(MapTile* mapTile, eDirection direction) const
{
    if (MapTile* neighbourTile = mapTile->mNeighbours[direction])
    {
        return mapTile->GetBaseTerrain() == neighbourTile->GetBaseTerrain();
    }
    return false;
}

bool DungeonBuilder::NeighbourRoomHandlesWalls(MapTile* mapTile, eDirection direction) const
{
    if (MapTile* neighbourTile = mapTile->mNeighbours[direction])
    {
        return RoomHandlesWalls(neighbourTile);
    }
    return false;
}

bool DungeonBuilder::RoomHandlesWalls(MapTile* mapTile) const
{
    if (mapTile == nullptr)
    {
        debug_assert(false);
        return false;
    }

    TerrainDefinition* terrain = mapTile->GetTerrain();
    if (gGameWorld.IsRoomTypeTerrain(terrain))
    {
        RoomDefinition* roomDefinition = gGameWorld.GetRoomDefinitionByTerrain(terrain);
        if (roomDefinition->HandlesWalls())
            return true;
    } 
    return false;
}

bool DungeonBuilder::NeighbourHasSameRoom(MapTile* mapTile, eDirection direction) const
{
    if (mapTile == nullptr || mapTile->mRoom == nullptr)
    {
        debug_assert(false);
        return false;
    }
    if (MapTile* neighbourTile = mapTile->mNeighbours[direction])
    {
        return mapTile->mRoom == neighbourTile->mRoom;
    }
    return false;
}
