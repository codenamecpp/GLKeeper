#include "pch.h"
#include "TempleRoom.h"
#include "RenderScene.h"
#include "WaterLavaMeshComponent.h"
#include "MapTile.h"
#include "GameObject.h"
#include "GameObjectsManager.h"
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
    if (mWaterPoolObject)
    {
        gRenderScene.DetachObject(mWaterPoolObject);
        gGameObjectsManager.DestroyGameObject(mWaterPoolObject);
    }
}

void TempleRoom::OnReconfigure()
{
    TilesArray waterTiles;
    ScanWaterPoolTiles(waterTiles);

    if (waterTiles.empty())
    {
        if (mWaterPoolObject)
        {
            gRenderScene.DetachObject(mWaterPoolObject);

            WaterLavaMeshComponent* component = mWaterPoolObject->GetComponent<WaterLavaMeshComponent>();
            debug_assert(component);

            component->ClearMesh();
        }
        return;
    }

    // create new water pool mesh
    if (mWaterPoolObject == nullptr)
    {
        mWaterPoolObject = gGameObjectsManager.CreateGameObject();

        WaterLavaMeshComponent* meshComponent = new WaterLavaMeshComponent(mWaterPoolObject);
        mWaterPoolObject->AddComponent(meshComponent);

        meshComponent->SetWaterLavaTiles(waterTiles);
        meshComponent->SetSurfaceTexture(gTexturesManager.mWaterTexture);
        meshComponent->SetSurfaceParams(TEMPLE_WATER_POOL_TRANSLUCENCY, TEMPLE_WATER_POOL_WAVE_WIDTH, 
            TEMPLE_WATER_POOL_WAVE_HEIGHT, TEMPLE_WATER_POOL_WAVE_FREQ, TEMPLE_WATER_POOL_WATERLINE);
        gRenderScene.AttachObject(mWaterPoolObject);

        return;
    }

    WaterLavaMeshComponent* meshComponent = mWaterPoolObject->GetComponent<WaterLavaMeshComponent>();
    debug_assert(meshComponent);

    if (!cxx::collections_equals(waterTiles, meshComponent->mWaterLavaTiles))
    {
        meshComponent->SetWaterLavaTiles(waterTiles);
    }

    if (!mWaterPoolObject->IsAttachedToScene())
    {
        gRenderScene.AttachObject(mWaterPoolObject);
    }
}

void TempleRoom::ScanWaterPoolTiles(TilesArray& poolTiles)
{
    poolTiles = mInnerTiles;
    for (MapTile* currentTile: mRoomTiles)
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
