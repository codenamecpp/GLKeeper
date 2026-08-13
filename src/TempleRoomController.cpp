#include "stdafx.h"
#include "TempleRoomController.h"
#include "GameRenderManager.h"
#include "GameWorld.h"
#include "Scene.h"

//////////////////////////////////////////////////////////////////////////

#define TEMPLE_WATER_POOL_TRANSLUCENCY  0.90f
#define TEMPLE_WATER_POOL_WAVE_WIDTH    4.0f
#define TEMPLE_WATER_POOL_WAVE_HEIGHT   0.08f
#define TEMPLE_WATER_POOL_WAVE_FREQ     16.2f
#define TEMPLE_WATER_POOL_WATERLINE     0.78f

//////////////////////////////////////////////////////////////////////////

void TempleRoomController::SpawnInstance()
{
    RoomController::SpawnInstance();
}

void TempleRoomController::DespawnInstance()
{
    RoomController::DespawnInstance();
    mWaterPool.reset();
    mWaterPoolTiles.clear();
    mHandLocation.reset();
}

void TempleRoomController::PostReconfigureRoom()
{
    ReevaluateHandLocation();

    ReevaluateWaterPoolTiles();

    if (!mWaterPoolTiles.empty())
    {
        // create water pool
        if (mWaterPool == nullptr)
        {
            mWaterPool = gScene.CreateWaterMesh(mWaterPoolTiles);

            if (mWaterPool)
            {
                EnvironmentMeshObject::Params surfaceParams;
                {
                    surfaceParams.mTranslucency = TEMPLE_WATER_POOL_TRANSLUCENCY;
                    surfaceParams.mWaveWidth = TEMPLE_WATER_POOL_WAVE_WIDTH;
                    surfaceParams.mWaveHeight = TEMPLE_WATER_POOL_WAVE_HEIGHT;
                    surfaceParams.mWaveFreq = TEMPLE_WATER_POOL_WAVE_FREQ;
                    surfaceParams.mWaterlineHeight = TEMPLE_WATER_POOL_WATERLINE;
                }
                mWaterPool->ConfigureParams(surfaceParams);
            }
        }
        else
        {
            mWaterPool->ConfigureMapTiles(mWaterPoolTiles);
        }

        cxx_assert(mWaterPool);

        if (mWaterPool)
        {
            mWaterPool->SetObjectActive(true);
        }
        else
        {
            gConsole.LogMessage(eLogLevel_Warning, "Cannot create temple water pool");
        }
    }
    else
    {
        DestroyWaterPool();
    }
}

void TempleRoomController::EvaluateFloorFurniture(RoomFurnitureSlots& evaluation)
{
    if (!mHandLocation.has_value()) return;

    RoomFurnitureSlot& objectSlot = evaluation.emplace_back();
    objectSlot.mTileLocation = *mHandLocation;
    objectSlot.mObjectClassId = GameObjectClassId_TempleHand;
}

void TempleRoomController::EvaluatePillars(RoomFurnitureSlots& evaluation)
{
    if (GetRoomPillarObjectId() == GameObjectClassId_Null)
        return;

    for (MapTile* roller: GetRoom().GetFloorTiles())
    {
        if (roller->mIsRoomInnerTile) continue;

        if (roller->SameNeighbourRoomInstance(eDirection_E) && roller->SameNeighbourRoomInstance(eDirection_W))
            continue;

        if (roller->SameNeighbourRoomInstance(eDirection_N) && roller->SameNeighbourRoomInstance(eDirection_S))
            continue;

        RoomFurnitureSlot& pillarSlot = evaluation.emplace_back();
        pillarSlot.mObjectClassId = GetRoomPillarObjectId();
        pillarSlot.mTileLocation = roller->mLocation;
    }
}

void TempleRoomController::ReevaluateWaterPoolTiles()
{
    mWaterPoolTiles.clear();
    for (MapTile* currTile: GetRoom().GetFloorTiles())
    {
        if (currTile->mIsRoomInnerTile)
        {
            mWaterPoolTiles.push_back(currTile);
            continue;
        }
   
        for (eDirection dir: gDirectionsCCW)
        {
            MapTile* neighbourTile = currTile->mNeighbours[dir];
            if (neighbourTile && neighbourTile->mIsRoomInnerTile)
            {
                mWaterPoolTiles.push_back(currTile);
                break;
            }
        }
    }
}

bool TempleRoomController::ReevaluateHandLocation()
{
    Temp_Vector<MapTile*> candidateTiles;

    auto roomInnerTiles = GetRoom().GetInnerTiles();

    candidateTiles.reserve(std::min(roomInnerTiles.size(), 64));

    for (MapTile* currTile: roomInnerTiles)
    {
        bool hasNeighbourBorders = false;
        for (eDirection dir: gDirectionsCCW)
        {
            MapTile* neighbourTile = currTile->mNeighbours[dir];
            if (neighbourTile && !neighbourTile->mIsRoomInnerTile)
            {
                hasNeighbourBorders = true;
                break;
            }
        }
        if (hasNeighbourBorders) continue;

        candidateTiles.push_back(currTile);
    }

    if (candidateTiles.empty())
    {
        mHandLocation.reset();
        return false;
    }

    // previous position still valid?
    MapPoint2D prevHandPosition = *mHandLocation;
    if (cxx::contains_if(candidateTiles, [prevHandPosition](MapTile* mapTile) { return mapTile->mLocation == prevHandPosition; }))
        return true;

    // set first valid
    mHandLocation = candidateTiles.front()->mLocation;
    return true;
}

void TempleRoomController::DestroyWaterPool()
{
    mWaterPool.reset();
}
