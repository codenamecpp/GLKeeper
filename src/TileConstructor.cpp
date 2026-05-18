#include "stdafx.h"
#include "TileConstructor.h"
#include "MapTile.h"
#include "GameMap.h"
#include "MeshAssetManager.h"
#include "GameWorld.h"

TileConstructor::TileConstructor(const ScenarioDefinition& scenarioDef)
    : mScenarioDef(scenarioDef)
{
}

void TileConstructor::ExtendTileMesh(MapTile* mapTile, eTileFace face, MeshAsset* mesh, const glm::mat3* rotation, const glm::vec3* translation)
{
    cxx_assert(mapTile);
    cxx_assert(mesh);
    cxx_assert(face < eTileFace_COUNT);

    // destination geometry
    TileFaceData& tileFace = mapTile->mFaces[face];
    const glm::vec3 tileTranslation = 
    {
        mapTile->mTileLocation.x + (translation ? translation->x : 0.0f), translation ? translation->y : 0.0f,
        mapTile->mTileLocation.y + (translation ? translation->z : 0.0f)
    };

    TerrainDefinition* terrainDefinition = mapTile->GetTerrain();

    // append submeshes
    for (const MeshAsset::SubMesh& pieceSprite: mesh->GetSubMeshList())
    {
        const MeshAsset::MaterialDesc& pieceMaterial = mesh->GetMaterialByIndex(pieceSprite.mMaterialIndex);

        Texture* diffuseTexture = pieceMaterial.mDiffuseTextures.front();
        Texture* envTexture = pieceMaterial.mEnvMappingTexture;

        if (terrainDefinition->mTextureFrames > 1)
        {
            const int numTextureVariations = pieceMaterial.mDiffuseTextures.size();
            if (numTextureVariations > 1)
            {
                switch (face)
                {
                    case eTileFace_SideN:
                    case eTileFace_SideE:
                    case eTileFace_SideS:
                    case eTileFace_SideW:
                    {
                        if (terrainDefinition->mIsDecay)
                        {
                            if (!tileFace.mWallExtendsRoom) // wall not specified
                            {
                                diffuseTexture = pieceMaterial.mDiffuseTextures.back();
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
                            const int ivariation = mapTile->mRandomValue % numTextureVariations; 
                            diffuseTexture = pieceMaterial.mDiffuseTextures[ivariation];
                            break;
                        }
                    }
                    break;
                }
            }  // numvariations
        }

        if (pieceSprite.mTris.empty())
        {
            // sometimes triangles is missed
            continue;
        }

        // add terrain mesh piece
        TileFaceMesh::Piece& tileMeshPiece = tileFace.mFaceMesh.mPieces.emplace_back();
        tileMeshPiece.mMaterial.mDiffuseTexture = diffuseTexture;
        tileMeshPiece.mMaterial.mEnvMappingTexture = envTexture;
        // force default render states for terrain geometries
        // alphablending leads to glitches
        tileMeshPiece.mMaterial.mRenderStates = RenderStates();
        tileMeshPiece.mBaseVertex = tileFace.mFaceMesh.mVertices.size();
        tileMeshPiece.mTrianglesOffset = tileFace.mFaceMesh.mTriangles.size();
        tileMeshPiece.mVertexCount = pieceSprite.mFrameVerticesCount;
        tileMeshPiece.mTriangleCount = pieceSprite.mTris.size();

        // source data
        const glm::vec3* sourcePositions = pieceSprite.mVertices.data();
        const glm::vec3* sourceNormals = pieceSprite.mNormals.data();
        const glm::vec2* sourceTexcoord = pieceSprite.mTexCoords.data();

        tileFace.mFaceMesh.mVertices.resize(tileMeshPiece.mBaseVertex + tileMeshPiece.mVertexCount);

        // transfrom vertex position
        if (rotation)
        {
            for (unsigned int ivertex = 0; ivertex < tileMeshPiece.mVertexCount; ++ivertex)
            {
                TerrainVertex3D& vertex = tileFace.mFaceMesh.mVertices[tileMeshPiece.mBaseVertex + ivertex];
                const glm::vec3 transformed = (*rotation * glm::vec3{
                    sourcePositions[ivertex].x, 
                    sourcePositions[ivertex].y, 
                    sourcePositions[ivertex].z});

                const glm::vec3 transformedNormal = (*rotation * glm::vec3{
                    sourceNormals[ivertex].x, 
                    sourceNormals[ivertex].y, 
                    sourceNormals[ivertex].z});

                vertex.mTileX = mapTile->mTileLocation.x;
                vertex.mTileY = mapTile->mTileLocation.y;
                vertex.mPosition = transformed + tileTranslation;
                vertex.mNormal = transformedNormal;
                vertex.mTexcoord = sourceTexcoord[ivertex];
            }
        }
        else // if rotation
        {
            for (unsigned int ivertex = 0; ivertex < tileMeshPiece.mVertexCount; ++ivertex)
            {
                TerrainVertex3D& vertex = tileFace.mFaceMesh.mVertices[tileMeshPiece.mBaseVertex + ivertex];
                vertex.mTileX = mapTile->mTileLocation.x;
                vertex.mTileY = mapTile->mTileLocation.y;
                vertex.mPosition = tileTranslation + sourcePositions[ivertex];
                vertex.mNormal = sourceNormals[ivertex];
                vertex.mTexcoord = sourceTexcoord[ivertex];
            }
        }
        // copy triangles unmodified
        tileFace.mFaceMesh.mTriangles.insert(tileFace.mFaceMesh.mTriangles.end(), 
            pieceSprite.mTris.begin(), 
            pieceSprite.mTris.end());
    }
}

void TileConstructor::ConstructTile(MapTile* mapTile)
{
    cxx_assert(mapTile);
    mapTile->ClearTileMesh();

    // construct floor
    ConstructFloor(mapTile);
    // construct walls
    if (mapTile->IsTerrainSolid())
    {
        ConstructWalls(mapTile);
    }
}

void TileConstructor::ConstructTile(MapTile* mapTile, eTileFace targetFace)
{
    mapTile->ClearTileMesh(targetFace);

    // construct floor
    if (targetFace == eTileFace_Floor)
    {
        ConstructFloor(mapTile);
    }

    // construct wall
    if ((targetFace == eTileFace_SideE) || (targetFace == eTileFace_SideN) || 
        (targetFace == eTileFace_SideS) || (targetFace == eTileFace_SideW))
    {
        if (mapTile->IsTerrainSolid())
        {
            ConstructWall(mapTile, targetFace);
        }
    }
    // ceiling not implemented yet
}

void TileConstructor::ConstructQuad(MapTile* mapTile, ArtResourceDefinition* resource)
{
    std::string meshName = resource->mResourceName;

    // coloured to player color
    TerrainDefinition* tileTerrainDef = mapTile->GetTerrain();
    if (tileTerrainDef->mPlayerColouredPath || tileTerrainDef->mPlayerColouredWall) 
    {
        static const char* playerIndices[] = {"0","0","1","2","3","4","5","6","7"};
        meshName.append(playerIndices[mapTile->mOwnerID]);
        meshName.append("_");
    }

    int subTiles[4] = {};
    if (mapTile->IsTerrainSolid())
    {
        //SubtTopLeft
        subTiles[0] = 
            (mapTile->NeighbourTileSolid(eDirection_W) ? 0x04 : 0) | 
            (mapTile->NeighbourTileSolid(eDirection_NW) ? 0x02 : 0) | 
            (mapTile->NeighbourTileSolid(eDirection_N) ? 0x01 : 0);
        //SubtTopRight
        subTiles[1] = 
            (mapTile->NeighbourTileSolid(eDirection_N) ? 0x04 : 0) | 
            (mapTile->NeighbourTileSolid(eDirection_NE) ? 0x02 : 0) | 
            (mapTile->NeighbourTileSolid(eDirection_E) ? 0x01 : 0);
        //SubtBottomRight
        subTiles[2] = 
            (mapTile->NeighbourTileSolid(eDirection_E) ? 0x04 : 0) | 
            (mapTile->NeighbourTileSolid(eDirection_SE) ? 0x02 : 0) | 
            (mapTile->NeighbourTileSolid(eDirection_S) ? 0x01 : 0);
        //SubtBottomLeft
        subTiles[3] = 
            (mapTile->NeighbourTileSolid(eDirection_S) ? 0x04 : 0) | 
            (mapTile->NeighbourTileSolid(eDirection_SW) ? 0x02 : 0) | 
            (mapTile->NeighbourTileSolid(eDirection_W) ? 0x01 : 0);
    }
    else
    {
        //SubtTopLeft
        subTiles[0] = 
            (mapTile->SameNeighbourTerrainType(eDirection_W) ? 0x04 : 0) | 
            (mapTile->SameNeighbourTerrainType(eDirection_NW) ? 0x02 : 0) | 
            (mapTile->SameNeighbourTerrainType(eDirection_N) ? 0x01 : 0);
        //SubtTopRight
        subTiles[1] = 
            (mapTile->SameNeighbourTerrainType(eDirection_N) ? 0x04 : 0) | 
            (mapTile->SameNeighbourTerrainType(eDirection_NE) ? 0x02 : 0) | 
            (mapTile->SameNeighbourTerrainType(eDirection_E) ? 0x01 : 0);
        //SubtBottomRight
        subTiles[2] = 
            (mapTile->SameNeighbourTerrainType(eDirection_E) ? 0x04 : 0) | 
            (mapTile->SameNeighbourTerrainType(eDirection_SE) ? 0x02 : 0) | 
            (mapTile->SameNeighbourTerrainType(eDirection_S) ? 0x01 : 0);
        //SubtBottomLeft
        subTiles[3] = 
            (mapTile->SameNeighbourTerrainType(eDirection_S) ? 0x04 : 0) | 
            (mapTile->SameNeighbourTerrainType(eDirection_SW) ? 0x02 : 0) | 
            (mapTile->SameNeighbourTerrainType(eDirection_W) ? 0x01 : 0);
    }

    const glm::mat3* rotations[] =
    {
        &g_TileRotations[2],
        &g_TileRotations[1],
        nullptr,
        &g_TileRotations[0]
    };

    // pieces
    MeshAsset* piece0 = gMeshAssetManager.GetMesh(meshName + "0");
    MeshAsset* piece1 = gMeshAssetManager.GetMesh(meshName + "1");
    MeshAsset* piece2 = gMeshAssetManager.GetMesh(meshName + "2");
    MeshAsset* piece3 = gMeshAssetManager.GetMesh(meshName + "3");
    MeshAsset* piece4 = gMeshAssetManager.GetMesh(meshName + "4");
    MeshAsset* pieces[8] = 
    {
        piece1, piece4, piece1, piece4, // 1, 4, 0, 4
        piece0, piece2, piece0, piece3
    };
    
    for (int isubtile = 0; isubtile < 4; ++isubtile)
    {
        ExtendTileFloor(mapTile, pieces[subTiles[isubtile]], rotations[isubtile], &g_SubTileTranslations[isubtile]);
    }
}

void TileConstructor::ConstructWall(MapTile* mapTile, eTileFace tileFace)
{
    cxx_assert(mapTile);

    // non solid tile cannot have walls
    if (!mapTile->IsTerrainSolid())
        return;

    eDirection wallDirection = TileFaceToDirection(tileFace);
    if (!IsStraightDirection(wallDirection))
    {
        cxx_assert(false);
        return;
    }

    // check neighbour
    if (mapTile->HasNeighbour(wallDirection))
    {
        // cannot construct wall next to solid block
        if (mapTile->NeighbourTileSolid(wallDirection)) return;
        // check room handles wall
        if (mapTile->IsTerrainAllowRoomWalls())
        {
            MapTile* neighbourTile = mapTile->mNeighbours[wallDirection];
            cxx_assert(neighbourTile);
            TerrainDefinition* neighbourTerrain = neighbourTile->GetTerrain();
            if (mScenarioDef.IsRoomTypeTerrain(neighbourTerrain))
            {
                RoomDefinition* roomDefinition = mScenarioDef.GetRoomDefinitionByTerrain(neighbourTerrain);
                if (roomDefinition->mHasWalls)
                    return;
            }
        }
    }

    const glm::mat3* rotation = nullptr;
    switch (tileFace)
    {
        case eTileFace_SideN: rotation = nullptr; break;
        case eTileFace_SideS: rotation = &g_TileRotations[2]; break;
        case eTileFace_SideE: rotation = &g_TileRotations[0]; break;
        case eTileFace_SideW: rotation = &g_TileRotations[1]; break;
    }
    TerrainDefinition* terrain = mapTile->GetTerrain();
    MeshAsset* piece = gMeshAssetManager.GetMesh(terrain->mResourceSide.mResourceName);
    ExtendTileMesh(mapTile, tileFace, piece, rotation);
}

void TileConstructor::ConstructFloor(MapTile* mapTile)
{
    TerrainDefinition* terrainDef = mapTile->GetTerrain();

    // if terrain is bridge we building only water bed
    if (mScenarioDef.IsRoomTypeTerrain(terrainDef))
    {
        if (mapTile->IsBaseTerrainWaterOrLava())
        {
            ConstructWaterBed(mapTile, mapTile->GetBaseTerrain()->GetCellResource());
        }
        // dont build room floor geometry here
        cxx_assert(mapTile->mRoomInstance);
        return;
    }

    ArtResourceDefinition* cellResource = terrainDef->GetCellResource();
    if (!cellResource || !cellResource->IsDefined())
        return;

    // water bed
    if (terrainDef->mConstructionTypeWater)
    {
        ConstructWaterBed(mapTile, cellResource);
    }
    // construct terrain quad
    else if (terrainDef->mConstructionTypeQuad)
    {
        ConstructQuad(mapTile, cellResource);
    }
    // construct terrain normal
    else
    {
        MeshAsset* resource = gMeshAssetManager.GetMesh(cellResource->mResourceName);
        ExtendTileFloor(mapTile, resource);
    }
}

void TileConstructor::ConstructWalls(MapTile* mapTile)
{
    for (eTileFace tileSide: {eTileFace_SideN, eTileFace_SideE, eTileFace_SideS, eTileFace_SideW})
    {
        ConstructWall(mapTile, tileSide);
    }
}

void TileConstructor::ConstructWaterBed(MapTile* mapTile, ArtResourceDefinition* resource)
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
        (!mapTile->SameNeighbourBaseTerrainType(eDirection_N) ? sDirsBits[eDirection_N] : 0U) | // top
        (!mapTile->SameNeighbourBaseTerrainType(eDirection_E) ? sDirsBits[eDirection_E] : 0U) | // right
        (!mapTile->SameNeighbourBaseTerrainType(eDirection_S) ? sDirsBits[eDirection_S] : 0U) | // bottom
        (!mapTile->SameNeighbourBaseTerrainType(eDirection_W) ? sDirsBits[eDirection_W] : 0U) | // left
    // corners
        (!mapTile->SameNeighbourBaseTerrainType(eDirection_NE) ? sDirsBits[eDirection_NE] : 0U) | // top-right
        (!mapTile->SameNeighbourBaseTerrainType(eDirection_SE) ? sDirsBits[eDirection_SE] : 0U) | // bottom-right
        (!mapTile->SameNeighbourBaseTerrainType(eDirection_SW) ? sDirsBits[eDirection_SW] : 0U) | // bottom-left
        (!mapTile->SameNeighbourBaseTerrainType(eDirection_NW) ? sDirsBits[eDirection_NW] : 0U); // top-left

    const unsigned char cornersBits = bits & 0xF0U;
    const unsigned char sidesBits = bits & 0x0FU;

    MeshAsset* piece0 = gMeshAssetManager.GetMesh(resource->mResourceName + "0");
    MeshAsset* piece1 = gMeshAssetManager.GetMesh(resource->mResourceName + "1");
    MeshAsset* piece2 = gMeshAssetManager.GetMesh(resource->mResourceName + "2");
    MeshAsset* piece3 = gMeshAssetManager.GetMesh(resource->mResourceName + "3");

    // simplest one
#pragma region SimplestOne
    if (bits == 0)
    {
        ExtendTileFloor(mapTile, piece3);
        return;
    }
#pragma endregion

    // corner
#pragma region Corner
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
                ExtendTileFloor(mapTile, piece2, pentry.second);
                break;
            }
        }
        return;
    }
#pragma endregion

    // side
#pragma region Side
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
                ExtendTileFloor(mapTile, piece0, pentry.second);
                break;
            }
        }
        return;
    }
#pragma endregion

    // land corner
#pragma region LandCorner
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
                ExtendTileFloor(mapTile, piece1, pentry.second);
                break;
            }
        }
        return;
    }
#pragma endregion

    int subTiles[] = {
    //SubtTopLeft
        (mapTile->SameNeighbourBaseTerrainType(eDirection_W) ? 0x04 : 0) | 
        (mapTile->SameNeighbourBaseTerrainType(eDirection_NW) ? 0x02 : 0) | 
        (mapTile->SameNeighbourBaseTerrainType(eDirection_N) ? 0x01 : 0),
    //SubtTopRight
        (mapTile->SameNeighbourBaseTerrainType(eDirection_N) ? 0x04 : 0) | 
        (mapTile->SameNeighbourBaseTerrainType(eDirection_NE) ? 0x02 : 0) | 
        (mapTile->SameNeighbourBaseTerrainType(eDirection_E) ? 0x01 : 0),
    //SubtBottomRight
        (mapTile->SameNeighbourBaseTerrainType(eDirection_E) ? 0x04 : 0) | 
        (mapTile->SameNeighbourBaseTerrainType(eDirection_SE) ? 0x02 : 0) | 
        (mapTile->SameNeighbourBaseTerrainType(eDirection_S) ? 0x01 : 0),
    //SubtBottomLeft
        (mapTile->SameNeighbourBaseTerrainType(eDirection_S) ? 0x04 : 0) | 
        (mapTile->SameNeighbourBaseTerrainType(eDirection_SW) ? 0x02 : 0) | 
        (mapTile->SameNeighbourBaseTerrainType(eDirection_W) ? 0x01 : 0)
    };

    MeshAsset* piece4 = gMeshAssetManager.GetMesh(resource->mResourceName + "4");
    MeshAsset* piece5 = gMeshAssetManager.GetMesh(resource->mResourceName + "5");
    MeshAsset* piece6 = gMeshAssetManager.GetMesh(resource->mResourceName + "6");
    MeshAsset* piece7 = gMeshAssetManager.GetMesh(resource->mResourceName + "7");
    MeshAsset* geoIndices[8] = {
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
        ExtendTileFloor(mapTile, geoIndices[subTiles[isubtile]], subtileRotations[isubtile][subTiles[isubtile]], &g_SubTileTranslations[isubtile]);
    }
}