#include "pch.h"
#include "GenericRoom.h"
#include "MapTile.h"
#include "ModelAssetsManager.h"
#include "DungeonBuilder.h"
#include "GameWorld.h"

GenericRoom::GenericRoom(RoomDefinition* definition, ePlayerID owner, RoomInstanceID uid)
    : mDefinition(definition)
    , mInstanceID(uid)
    , mOwnerIdentifier(owner)
{
    debug_assert(mDefinition);
}

GenericRoom::~GenericRoom()
{
}

void GenericRoom::UpdateFrame()
{

}

void GenericRoom::ReleaseTiles(const TilesArray& mapTiles)
{
}

void GenericRoom::ReleaseTiles()
{
}

void GenericRoom::EnlargeRoom(const TilesArray& mapTiles)
{
    IncludeTiles(mapTiles);
    // update bounds and inner squares
    ReevaluateOccupationArea();
    ReevaluateInnerSquares();
}

void GenericRoom::BuildTilesMesh()
{
    ConstructFloorTiles(gGameWorld.mDungeonBuilder, mRoomTiles);
}

void GenericRoom::UpdateTilesMesh()
{
    
}

void GenericRoom::IncludeTiles(const TilesArray& mapTiles)
{
    // first scan all good tiles and assign room instance to them
    for (MapTile* currTile: mapTiles)
    {
        debug_assert(currTile->mIsRoomEntrance == false);
        debug_assert(currTile->mIsRoomInnerTile == false);
        debug_assert(currTile->mRoom == nullptr);

        currTile->mRoom = this;
#ifdef _DEBUG
        // check no room walls
        if (MapTile* neighTile = currTile->mNeighbours[eDirection_N]) { debug_assert(neighTile->mFaces[eTileFace_SideS].mRoomWallSection == nullptr); }
        if (MapTile* neighTile = currTile->mNeighbours[eDirection_E]) { debug_assert(neighTile->mFaces[eTileFace_SideW].mRoomWallSection == nullptr); }
        if (MapTile* neighTile = currTile->mNeighbours[eDirection_S]) { debug_assert(neighTile->mFaces[eTileFace_SideN].mRoomWallSection == nullptr); }
        if (MapTile* neighTile = currTile->mNeighbours[eDirection_W]) { debug_assert(neighTile->mFaces[eTileFace_SideE].mRoomWallSection == nullptr); }
#endif
        // invalidate tiles
        currTile->InvalidateTileMesh();
        currTile->InvalidateNeighbourTilesMesh();
        
        mRoomTiles.push_back(currTile);
    }
}

void GenericRoom::ReevaluateOccupationArea()
{
    // room is empty
    if (mRoomTiles.empty())
    {
        mOccupationArea.SetNull();
        return;
    }

    Point2D rightBottomPoint = mRoomTiles[0]->mTileLocation;
    Point2D leftTopPoint = mRoomTiles[0]->mTileLocation;

    for (MapTile* tile : mRoomTiles)
    {
        if (tile->mTileLocation.x < leftTopPoint.x) leftTopPoint.x = tile->mTileLocation.x;
        else rightBottomPoint.x = tile->mTileLocation.x;

        if (tile->mTileLocation.y < leftTopPoint.y) leftTopPoint.y = tile->mTileLocation.y;
        else rightBottomPoint.y = tile->mTileLocation.y;
    }

    mOccupationArea.mX = leftTopPoint.x;
    mOccupationArea.mY = leftTopPoint.y;
    mOccupationArea.mSizeX = (rightBottomPoint.x > 0) ? (rightBottomPoint.x - leftTopPoint.x + 1) : 1;
    mOccupationArea.mSizeY = (rightBottomPoint.y > 0) ? (rightBottomPoint.y - leftTopPoint.y + 1) : 1;
}

void GenericRoom::ReevaluateInnerSquares()
{
    auto CheckIsInnerTile = [this](DungeonBuilder& builder, MapTile* currTile) -> bool
    {
        int numPassed = 0;
        for (eDirection direction: gDirectionsCW)
        {
            bool isSameRoom = builder.NeighbourHasSameRoom(currTile, direction);
            if (!isSameRoom)
            {
                break;
            }
            ++numPassed;
        }
        return (numPassed == eDirection_COUNT);
    };

    mInnerTiles.clear();

    for (MapTile* currTile : mRoomTiles)
    {
        bool isInnerTile = CheckIsInnerTile(gGameWorld.mDungeonBuilder, currTile);
        if (isInnerTile != currTile->mIsRoomInnerTile)
        {
            // invalidate tiles
            currTile->InvalidateTileMesh();
            currTile->InvalidateNeighbourTilesMesh();
        }
        currTile->mIsRoomInnerTile = isInnerTile;
        if (isInnerTile)
        {
            mInnerTiles.push_back(currTile);
        }
    }
}

void GenericRoom::ConstructFloorTiles(DungeonBuilder& builder, const TilesArray& mapTiles)
{
    switch (mDefinition->mTileConstruction)
    {
        case eRoomTileConstruction_5_by_5_Rotated:
            ConstructTiles_5x5Rotated(builder, mapTiles); 
        break;

        case eRoomTileConstruction_3_by_3:
            ConstructTiles_3x3(builder, mapTiles); 
        break;

        case eRoomTileConstruction_Quad:
            ConstructTiles_Quad(builder, mapTiles); 
        break;

        case eRoomTileConstruction_Normal:
            ConstructTiles_Normal(builder, mapTiles); 
        break;

        case eRoomTileConstruction_DoubleQuad:
            ConstructTiles_DoubleQuad(builder, mapTiles); 
        break;

        case eRoomTileConstruction_HeroGateFrontEnd:
            ConstructTiles_HeroGateFrontEnd(builder, mapTiles);
        break;

        case eRoomTileConstruction_Complete:
        {
            int bp = 0; // todo
        }
        break;

        case eRoomTileConstruction_3_by_3_Rotated:
        {
            int bp = 0; // todo
        }
        break;

        case eRoomTileConstruction_CenterPool:
        {
            int bp = 0; // todo
        }
        break;

        case eRoomTileConstruction_HeroGate:
        {
            int bp = 0; // todo
        }
        break;

        case eRoomTileConstruction_HeroGateTile:
        {
            int bp = 0; // todo
        }
        break;

        case eRoomTileConstruction_HeroGate_2_by_2:
        {
            int bp = 0; // todo
        }
        break;

        case eRoomTileConstruction_HeroGate_3_by_1:
            ConstructTiles_HeroGate3x1(builder, mapTiles);
        break;
    }
}

void GenericRoom::ConstructTiles_3x3(DungeonBuilder& builder, const TilesArray& mapTiles)
{
    ModelAsset* pieces[] = 
    {
        gModelsManager.LoadModelAsset(mDefinition->mCompleteResource.mResourceName + "0"),
        gModelsManager.LoadModelAsset(mDefinition->mCompleteResource.mResourceName + "1"),
        gModelsManager.LoadModelAsset(mDefinition->mCompleteResource.mResourceName + "2"),
        gModelsManager.LoadModelAsset(mDefinition->mCompleteResource.mResourceName + "3"),
        gModelsManager.LoadModelAsset(mDefinition->mCompleteResource.mResourceName + "4"),
        gModelsManager.LoadModelAsset(mDefinition->mCompleteResource.mResourceName + "5"),
        gModelsManager.LoadModelAsset(mDefinition->mCompleteResource.mResourceName + "6"),
        gModelsManager.LoadModelAsset(mDefinition->mCompleteResource.mResourceName + "7"),
        gModelsManager.LoadModelAsset(mDefinition->mCompleteResource.mResourceName + "8"),
    };

    for (MapTile* currTile : mapTiles)
    {
        const int ioffsetx = currTile->mTileLocation.x - mOccupationArea.mX;
        const int ioffsety = currTile->mTileLocation.y - mOccupationArea.mY;
        const int ioffset = (ioffsety * 3) + ioffsetx;

        debug_assert(ioffset >= 0 && ioffset <= 8);
        if (ioffset < 0 || ioffset > 8)
            return;

        builder.ExtendTileMesh(currTile, eTileFace_Floor, pieces[ioffset]);
    }
}

void GenericRoom::ConstructTiles_Quad(DungeonBuilder& builder, const TilesArray& mapTiles)
{
    const std::string& meshName = mDefinition->mCompleteResource.mResourceName;

    // rotation shortcuts
    const glm::mat3* Neg90 = &g_TileRotations[0];
    const glm::mat3* Pos90 = &g_TileRotations[1];
    const glm::mat3* NegPi = &g_TileRotations[2];
    const glm::mat3* Nullp = nullptr;

    static const glm::mat3* subtileRotations[][8] = {
        {Pos90, Pos90, Pos90, Pos90, NegPi, Neg90, NegPi, Pos90},
        {Nullp, Nullp, Nullp, Nullp, Pos90, NegPi, Pos90, Nullp},
        {Neg90, Neg90, Neg90, Neg90, Nullp, Pos90, Nullp, Neg90},
        {NegPi, NegPi, NegPi, NegPi, Neg90, Nullp, Neg90, NegPi},
    };

    ModelAsset* geoPiece0 = gModelsManager.LoadModelAsset(meshName + "0");
    ModelAsset* geoPiece1 = gModelsManager.LoadModelAsset(meshName + "1");
    ModelAsset* geoPiece2 = gModelsManager.LoadModelAsset(meshName + "2");

    ModelAsset* geometryPieces[8] = 
    {
        geoPiece1, geoPiece0, geoPiece1, geoPiece0,
        geoPiece0, geoPiece2, geoPiece0, geoPiece0
    };

    for (MapTile* currTile : mapTiles)
    {
        int subTiles[] = 
        {
        //SubtTopLeft
            (builder.NeighbourHasSameRoom(currTile, eDirection_W) ? 0x04 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_NW) ? 0x02 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_N) ? 0x01 : 0),
        //SubtTopRight
            (builder.NeighbourHasSameRoom(currTile, eDirection_N) ? 0x04 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_NE) ? 0x02 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_E) ? 0x01 : 0),
        //SubtBottomRight
            (builder.NeighbourHasSameRoom(currTile, eDirection_E) ? 0x04 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_SE) ? 0x02 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_S) ? 0x01 : 0),
        //SubtBottomLeft
            (builder.NeighbourHasSameRoom(currTile, eDirection_S) ? 0x04 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_SW) ? 0x02 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_W) ? 0x01 : 0)
        };
    
        for (int isubtile = 0; isubtile < 4; ++isubtile)
        {
            builder.ExtendTileMesh(currTile, eTileFace_Floor, geometryPieces[subTiles[isubtile]],
                subtileRotations[isubtile][subTiles[isubtile]], &g_SubTileTranslations[isubtile]);
        }
    }
}

void GenericRoom::ConstructTiles_Normal(DungeonBuilder& builder, const TilesArray& mapTiles)
{
    const std::string& meshName = mDefinition->mCompleteResource.mResourceName;

    // geometries
    ModelAsset* geoInnerPart = gModelsManager.LoadModelAsset(meshName + "9");
    ModelAsset* geoPiece0 = gModelsManager.LoadModelAsset(meshName + "0");
    ModelAsset* geoPiece1 = gModelsManager.LoadModelAsset(meshName + "1");
    ModelAsset* geoPiece2 = gModelsManager.LoadModelAsset(meshName + "2");
    ModelAsset* geoPiece3 = gModelsManager.LoadModelAsset(meshName + "3"); // inner
    ModelAsset* pieces[] =
    {
        geoPiece1, // !SideA && !Corner && !SideB
        geoPiece0, // SideB
        geoPiece1, // Corner
        geoPiece0, // SideB && Corner
        geoPiece0, // SideA
        geoPiece2, // SideA && SideB
        geoPiece0, // SideA && Corner
        geoPiece3, // SideA && Corner && SideB
    };

    // rotation shortcuts
    const glm::mat3* Neg90 = &g_TileRotations[0];
    const glm::mat3* Pos90 = &g_TileRotations[1];
    const glm::mat3* NegPi = &g_TileRotations[2];
    const glm::mat3* Nullp = nullptr;

    const glm::mat3* pieceRotations[][8] = {
        {Pos90, Pos90, Pos90, Pos90, Nullp, Neg90, Nullp, Nullp},
        {Nullp, Nullp, Nullp, Nullp, Neg90, NegPi, Neg90, Neg90},
        {Neg90, Neg90, Neg90, Neg90, NegPi, Pos90, NegPi, Nullp},
        {NegPi, NegPi, NegPi, NegPi, Pos90, Nullp, Pos90, Pos90}
    };

    for (MapTile* currTile : mapTiles) 
    {
        // todo: fix it
        // todo: what i had to fix here?
        if (currTile->mRoom == this && currTile->mIsRoomInnerTile)
        {
            builder.ExtendTileMesh(currTile, eTileFace_Floor, geoInnerPart);
            continue;
        }

        int subTiles[] = 
        {
        //SubtTopLeft
            (builder.NeighbourHasSameRoom(currTile, eDirection_W) ? 0x04 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_NW) ? 0x02 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_N) ? 0x01 : 0),
        //SubtTopRight
            (builder.NeighbourHasSameRoom(currTile, eDirection_N) ? 0x04 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_NE) ? 0x02 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_E) ? 0x01 : 0),
        //SubtBottomRight
            (builder.NeighbourHasSameRoom(currTile, eDirection_E) ? 0x04 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_SE) ? 0x02 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_S) ? 0x01 : 0),
        //SubtBottomLeft
            (builder.NeighbourHasSameRoom(currTile, eDirection_S) ? 0x04 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_SW) ? 0x02 : 0) | 
            (builder.NeighbourHasSameRoom(currTile, eDirection_W) ? 0x01 : 0)
        };

        for (int isubtile = 0; isubtile < 4; ++isubtile)
        {
            builder.ExtendTileMesh(currTile, eTileFace_Floor, pieces[subTiles[isubtile]],
                pieceRotations[isubtile][subTiles[isubtile]], &g_SubTileTranslations[isubtile]);
        }
    }
}

void GenericRoom::ConstructTiles_HeroGateFrontEnd(DungeonBuilder& builder, const TilesArray& mapTiles)
{    
    const std::string& meshName = mDefinition->mCompleteResource.mResourceName;
    for (MapTile* currTile: mapTiles)
    {
        int tilex = currTile->mTileLocation.x - mOccupationArea.mX;
        int tiley = currTile->mTileLocation.y - mOccupationArea.mY;
        if (tiley > 4)
        {
            tilex = 0;
        }
        int imesh = tiley * 3 + tilex;
       
        ModelAsset* meshResource = gModelsManager.LoadModelAsset(meshName + std::to_string(imesh + 1));
        debug_assert(meshResource);
        if (meshResource)
        {
            builder.ExtendTileMesh(currTile, eTileFace_Floor, meshResource);
        }
    }
}

void GenericRoom::ConstructTiles_HeroGate3x1(DungeonBuilder& builder, const TilesArray& mapTiles)
{
    // cannot build until things will be parsed, need info about direction
}

void GenericRoom::ConstructTiles_DoubleQuad(DungeonBuilder& builder, const TilesArray& mapTiles)
{
    const std::string& meshName = mDefinition->mCompleteResource.mResourceName;

    // geometries
    ModelAsset* geoInnerPart = gModelsManager.LoadModelAsset(meshName + "9");
    ModelAsset* geoPiece0 = gModelsManager.LoadModelAsset(meshName + "0");
    ModelAsset* geoPiece1 = gModelsManager.LoadModelAsset(meshName + "1");
    ModelAsset* geoPiece2 = gModelsManager.LoadModelAsset(meshName + "2");
    ModelAsset* geoPiece3 = gModelsManager.LoadModelAsset(meshName + "3"); // inner
    ModelAsset* innerGeoPiece0 = gModelsManager.LoadModelAsset(meshName + "10");
    ModelAsset* innerGeoPiece1 = gModelsManager.LoadModelAsset(meshName + "11");
    ModelAsset* innerGeoPiece2 = gModelsManager.LoadModelAsset(meshName + "12");
    ModelAsset* innerGeoPiece3 = gModelsManager.LoadModelAsset(meshName + "13"); // inner
    ModelAsset* geoEntrance = gModelsManager.LoadModelAsset(meshName + "14"); // entrance
    ModelAsset* pieces[] =
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

    for (MapTile* currTile : mapTiles)
    {
        // entrance
        if (currTile->mIsRoomEntrance)
        {
            builder.ExtendTileMesh(currTile, eTileFace_Floor, geoEntrance, nullptr, &g_SubTileTranslations[0]);
            continue;
        }

        // inner tile
        if (currTile->mIsRoomInnerTile)
        {
            builder.ExtendTileMesh(currTile, eTileFace_Floor, innerGeoPiece3, nullptr,             &g_SubTileTranslations[0]);
            builder.ExtendTileMesh(currTile, eTileFace_Floor, innerGeoPiece3, &g_TileRotations[0], &g_SubTileTranslations[1]);
            builder.ExtendTileMesh(currTile, eTileFace_Floor, innerGeoPiece3, &g_TileRotations[2], &g_SubTileTranslations[2]);
            builder.ExtendTileMesh(currTile, eTileFace_Floor, innerGeoPiece3, &g_TileRotations[1], &g_SubTileTranslations[3]);

            continue;
        }

        MapTile* neighbours[eDirection_COUNT] = {};
        for (eDirection direction: gDirectionsCW)
        {
            if (builder.NeighbourHasSameRoom(currTile, direction))
            {
                neighbours[direction] = currTile->mNeighbours[direction];
            }
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
                builder.ExtendTileMesh(currTile, eTileFace_Floor, pieces[exSubTiles[isubtile] + 8], 
                    innerPieceRotations[isubtile][exSubTiles[isubtile]], &g_SubTileTranslations[isubtile]);
            }
            else
            {
                builder.ExtendTileMesh(currTile, eTileFace_Floor, pieces[subTiles[isubtile]], 
                    pieceRotations[isubtile][subTiles[isubtile]], &g_SubTileTranslations[isubtile]);
            }
        }

        #undef IS_INNER
    }
}

void GenericRoom::ConstructTiles_5x5Rotated(DungeonBuilder& builder, const TilesArray& mapTiles)
{
    const std::string& meshName = mDefinition->mCompleteResource.mResourceName;
    for (MapTile* currTile : mapTiles)
    {
        const glm::mat3* rotation = nullptr;

        // get tile offset
        const int ioffsetx = currTile->mTileLocation.x - mOccupationArea.mX;
        const int ioffsety = currTile->mTileLocation.y - mOccupationArea.mY;
        const int ioffset = (ioffsety * 5) + ioffsetx;

        // corner
        bool isCorner = 
            ((ioffsetx == 0) || (ioffsetx == 4)) &&
            ((ioffsety == 0) || (ioffsety == 4));

        if (isCorner)
        {
            ModelAsset* geometry = gModelsManager.LoadModelAsset(meshName + "3");
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

            builder.ExtendTileMesh(currTile, eTileFace_Floor, geometry, rotation);
            continue;
        }

        // Outer layer sides
        bool isOuterLayerSides =
            (ioffsetx == 0) || (ioffsetx == 4) ||
            (ioffsety == 0) || (ioffsety == 4);

        if (isOuterLayerSides)
        {
            ModelAsset* geometry = gModelsManager.LoadModelAsset(meshName + "2");
            if (ioffsetx == 0 || ioffsetx == 4)
            {
                rotation = (ioffsetx == 0) ? &g_TileRotations[2] : nullptr;
            }
            else
            {
                rotation = (ioffsety == 0) ? &g_TileRotations[1] : &g_TileRotations[0];
            }

            builder.ExtendTileMesh(currTile, eTileFace_Floor, geometry, rotation);
            continue;
        }

        // The inner ring, corners
        bool isInnerCorner =
            ((ioffsetx == 1) || (ioffsetx == 3)) &&
            ((ioffsety == 1) || (ioffsety == 3));

        if (isInnerCorner)
        {
            ModelAsset* geometry = gModelsManager.LoadModelAsset(meshName + "0");
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

            builder.ExtendTileMesh(currTile, eTileFace_Floor, geometry, rotation);
            continue;
        }

        // The inner ring, sides
        bool isInnerSides =
            (ioffsetx == 1) || (ioffsetx == 3) ||
            (ioffsety == 1) || (ioffsety == 3);

        if (isInnerSides)
        {
            ModelAsset* geometry = gModelsManager.LoadModelAsset(meshName + "1");
            if (ioffsetx == 1 || ioffsetx == 3)
            {
                rotation = (ioffsetx == 1) ? &g_TileRotations[1] : &g_TileRotations[0];
            }
            else
            {
                rotation = (ioffsety == 1) ? nullptr : &g_TileRotations[2];
            }

            builder.ExtendTileMesh(currTile, eTileFace_Floor, geometry, rotation);
            continue;
        }
    } // for mapTile
}