#include "pch.h"
#include "TempleRoom.h"
#include "RenderScene.h"
#include "RenderableWaterLavaMesh.h"
#include "TerrainTile.h"
#include "SceneObject.h"
#include "TexturesManager.h"

#define TEMPLE_WATER_POOL_TRANSLUCENCY  0.90f
#define TEMPLE_WATER_POOL_WAVE_WIDTH    4.0f
#define TEMPLE_WATER_POOL_WAVE_HEIGHT   0.08f
#define TEMPLE_WATER_POOL_WAVE_FREQ     16.2f
#define TEMPLE_WATER_POOL_WATERLINE     0.78f

TempleRoom::TempleRoom(RoomDefinition* definition, ePlayerID owner, RoomInstanceID uid)
    : GenericRoom(definition, owner, uid)
{
}

TempleRoom::~TempleRoom()
{
    if (mWaterPool)
    {
        mWaterPool->DestroyObject();
        mWaterPool = nullptr;
    }
}

void TempleRoom::Reconfigure()
{
    TilesList waterTiles;
    ScanWaterPoolTiles(waterTiles);

    if (waterTiles.empty())
    {
        if (mWaterPool)
        {
            mWaterPool->SetActive(false);
        }
        return;
    }

    // create new water pool mesh
    if (mWaterPool == nullptr)
    {
        mWaterPool = new RenderableWaterLavaMesh;

        mWaterPool->SetWaterLavaTiles(waterTiles);
        mWaterPool->SetSurfaceTexture(gTexturesManager.mWaterTexture);
        mWaterPool->SetSurfaceParams(TEMPLE_WATER_POOL_TRANSLUCENCY, TEMPLE_WATER_POOL_WAVE_WIDTH, 
            TEMPLE_WATER_POOL_WAVE_HEIGHT, TEMPLE_WATER_POOL_WAVE_FREQ, TEMPLE_WATER_POOL_WATERLINE);
        return;
    }

    if (!cxx::collections_equals(waterTiles, mWaterPool->mWaterLavaTiles))
    {
        mWaterPool->SetWaterLavaTiles(waterTiles);
    }

    mWaterPool->SetActive(true);
}

void TempleRoom::ScanWaterPoolTiles(TilesList& poolTiles)
{
    poolTiles = mInnerTiles;
    for (TerrainTile* currentTile: mRoomTiles)
    {
        if (currentTile->mIsRoomInnerTile)
            continue;

        #define IS_INNER(direction) \
            currentTile->mNeighbours[direction]->mIsRoomInnerTile  

        for (eDirection dir: gDirectionsCCW)
        {
            if (IS_INNER(dir))
            {
                poolTiles.push_back(currentTile);
                break;
            }
        }
        #undef IS_INNER
    }
}
