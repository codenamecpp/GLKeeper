#include "stdafx.h"
#include "HeroGateFrontendRoomController.h"
#include "MapTile.h"
#include "GameObject.h"
#include "GameMain.h"
#include "GameWorldDefs.h"
#include "GameWorld.h"
#include "AnimatingMeshObject.h"
#include "MeshAssetManager.h"
#include "GameObjectManager.h"
#include "Scene.h"
#include "MapUtils.h"
#include "EntityRegistry.h"

//////////////////////////////////////////////////////////////////////////

#define FRONT_END_BEAMS_EFFECT_MESH "3dfe_beams"
#define FRONT_END_LEVELS_TABLE_MESH "3dmap_level21"

//////////////////////////////////////////////////////////////////////////

void HeroGateFrontendRoomController::PostReconfigureRoom()
{
    Init3dMap();
}

void HeroGateFrontendRoomController::SpawnInstance()
{
    RoomController::SpawnInstance();

    GetRoom().AddCapability<I3DMapTableRoom>(this);
}

void HeroGateFrontendRoomController::DespawnInstance()
{
    RoomController::DespawnInstance();

    for (const auto& roller: m3dMapTablePieces)
    {
        cxx_assert(roller.first.IsGameObject());
        gEntities.DeleteEntity(roller.first);
    }
    m3dMapTablePieces.clear();
    mSelectedTablePiece = {};
}

void HeroGateFrontendRoomController::OnRecycle()
{
    RoomController::OnRecycle();
    m3dMapTablePieces.clear();
    mSelectedTablePiece = {};
}

void HeroGateFrontendRoomController::EvaluateFloorFurniture(RoomFurnitureSlots& evaluation)
{
    const Rect2D& locationArea = GetRoom().GetLocationArea();

    Point2D baseLocation {locationArea.x, locationArea.y};

    auto PutObject = [&evaluation](const Point2D& tileLocation, GameObjectClassId objectId)
        {
            RoomFurnitureSlot& objectSlot = evaluation.emplace_back();
            objectSlot.mObjectClassId = objectId;
            objectSlot.mTileLocation = tileLocation;
        };

    auto PutCandle = [&evaluation](const Point2D& tileLocation, RoomFurnitureSlot::eFaceRotation rotation)
        {
            RoomFurnitureSlot& objectSlot = evaluation.emplace_back();
            objectSlot.mObjectClassId = GameObjectClassId_3DFrontEndChain;
            objectSlot.mObjectRotation = rotation;
            objectSlot.mTileLocation = tileLocation;
        };

    PutObject({baseLocation.x + 1, baseLocation.y}, GameObjectClassId_3DFrontEndBanner1);
    PutObject({baseLocation.x + 1, baseLocation.y}, GameObjectClassId_3DFrontEndBanner2);
    PutCandle({baseLocation.x + 1, baseLocation.y}, RoomFurnitureSlot::eFaceRotation_0);
    PutCandle({baseLocation.x + 1, baseLocation.y + 1}, RoomFurnitureSlot::eFaceRotation_180);

    PutObject({baseLocation.x + 1, baseLocation.y + 1}, GameObjectClassId_3DFrontEndBanner3);
    PutObject({baseLocation.x + 1, baseLocation.y + 1}, GameObjectClassId_3DFrontEndBanner4);
    PutCandle({baseLocation.x + 1, baseLocation.y + 1}, RoomFurnitureSlot::eFaceRotation_0);
    PutCandle({baseLocation.x + 1, baseLocation.y + 2}, RoomFurnitureSlot::eFaceRotation_180);

    PutObject({baseLocation.x + 1, baseLocation.y + 2}, GameObjectClassId_3DFrontEndBanner1);
    PutObject({baseLocation.x + 1, baseLocation.y + 2}, GameObjectClassId_3DFrontEndBanner2);
    PutCandle({baseLocation.x + 1, baseLocation.y + 2}, RoomFurnitureSlot::eFaceRotation_0);
    PutCandle({baseLocation.x + 1, baseLocation.y + 3}, RoomFurnitureSlot::eFaceRotation_180);

    PutObject({baseLocation.x + 1, baseLocation.y + 3}, GameObjectClassId_3DFrontEndBanner3);
    PutObject({baseLocation.x + 1, baseLocation.y + 3}, GameObjectClassId_3DFrontEndBanner4);
    PutCandle({baseLocation.x + 1, baseLocation.y + 3}, RoomFurnitureSlot::eFaceRotation_0);
    PutCandle({baseLocation.x + 1, baseLocation.y + 4}, RoomFurnitureSlot::eFaceRotation_180);

    PutObject({baseLocation.x + 1, baseLocation.y}, GameObjectClassId_3DFrontEndGemHolder);
}

void HeroGateFrontendRoomController::PostRearrangeObjects()
{
    GameObjectManager& gobjects = gGameObjectManager;
    for (const RoomFurnitureSlot& roller: GetRoom().GetFloorFurniture())
    {
        if (GameObject* gameObject = gobjects.GetObjectPtr(roller.mObjectHandle))
        {
            if (AnimatingMeshObject* meshObject = gameObject->GetMeshObject())
            {
                meshObject->SetAnimationProgress(Random::GenerateFloat01());
            }
        }
    }
}

bool HeroGateFrontendRoomController::IsLevel(EntityHandle objectHandle) const
{
    return objectHandle && cxx::contains_if(m3dMapTablePieces, 
        [objectHandle](const TablePieceEntry& entry)
        {
            return (entry.first == objectHandle) && entry.second;
        });
}

bool HeroGateFrontendRoomController::SelectLevel(EntityHandle objectHandle)
{
    if (!IsLevel(objectHandle))
        return false;

    if (mSelectedTablePiece != objectHandle)
    {
        DeselectLevel();
        mSelectedTablePiece = objectHandle;

        if (auto capability = gEntities.GetEntityCapability<I3DMapTableLevel>(mSelectedTablePiece))
        {
            capability->SelectLevel();
        }
    }
    return true;
}

void HeroGateFrontendRoomController::DeselectLevel()
{
    if (!mSelectedTablePiece)
        return;

    if (auto capability = gEntities.GetEntityCapability<I3DMapTableLevel>(mSelectedTablePiece))
    {
        capability->DeselectLevel();
    }
    mSelectedTablePiece = {};
}

bool HeroGateFrontendRoomController::GetLevelKey(EntityHandle objectHandle, CampaignLevelKey& levelKey) const
{
    const int entryIndex = cxx::get_index_if(m3dMapTablePieces, 
        [objectHandle](const TablePieceEntry& entry)
        {
            return (entry.first == objectHandle) && entry.second;
        });
    bool isSuccess = (entryIndex != -1);
    if (isSuccess)
    {
        levelKey = m3dMapTablePieces[entryIndex].second;
    }
    return isSuccess;
}

void HeroGateFrontendRoomController::Init3dMap()
{
    bool isInited = !m3dMapTablePieces.empty();

    if (isInited)
        return;

    const Point2D tableTileLocation = GetRoom().GetLocationArea().GetPosition() + Point2D {1, 3};
    const glm::vec3 tablePosition = MapUtils::ComputeTileCenter(tableTileLocation);

    m3dMapTablePieces.reserve(36);

    auto createTablePiece = [this, &tablePosition](const CampaignLevelKey& levelKey, const std::string& meshName)
        {
            EntityHandle tablePiece = gGameObjectManager.CreateObject(GameObjectClassId_3DFrontEndHeroGateTable);
            bool isSucces = gGameObjectManager.ActivateObject(tablePiece);
            if (isSucces)
            {
                this->m3dMapTablePieces.emplace_back(tablePiece, levelKey);
            }
            cxx_assert(isSucces);
            // post configure
            if (GameObject* objectPtr = gGameObjectManager.GetObjectPtr(tablePiece))
            {
                objectPtr->ParentRoomChanged(this->GetRoom().GetOwnHandle());
                objectPtr->SetCustomMesh(meshName);
                objectPtr->SetPosition(tablePosition);
            }
        };
    createTablePiece({},                                FRONT_END_LEVELS_TABLE_MESH);
    createTablePiece({eCampaignLevelType_Main, 1},      "3dmap_level1");
    createTablePiece({eCampaignLevelType_Main, 2},      "3dmap_level2");
    createTablePiece({eCampaignLevelType_Main, 3},      "3dmap_level3");
    createTablePiece({eCampaignLevelType_Main, 4},      "3dmap_level4");
    createTablePiece({eCampaignLevelType_Main, 5},      "3dmap_level5");
    //"3dmap_level6",
    createTablePiece({eCampaignLevelType_Main, 6,'a'},  "3dmap_level6a");
    createTablePiece({eCampaignLevelType_Main, 6,'b'},  "3dmap_level6b");
    createTablePiece({eCampaignLevelType_Main, 7},      "3dmap_level7");
    createTablePiece({eCampaignLevelType_Main, 8},      "3dmap_level8");
    createTablePiece({eCampaignLevelType_Main, 9},      "3dmap_level9");
    createTablePiece({eCampaignLevelType_Main,10},      "3dmap_level10");
    //"3dmap_level11",
    createTablePiece({eCampaignLevelType_Main,11,'a'},  "3dmap_level11a");
    createTablePiece({eCampaignLevelType_Main,11,'b'},  "3dmap_level11b");
    createTablePiece({eCampaignLevelType_Main,11,'c'},  "3dmap_level11c");
    createTablePiece({eCampaignLevelType_Main,12},      "3dmap_level12");
    createTablePiece({eCampaignLevelType_Main,13},      "3dmap_level13");
    createTablePiece({eCampaignLevelType_Main,14},      "3dmap_level14");
    //"3dmap_level15",
    createTablePiece({eCampaignLevelType_Main,15,'a'},  "3dmap_level15a");
    createTablePiece({eCampaignLevelType_Main,15,'b'},  "3dmap_level15b");
    createTablePiece({eCampaignLevelType_Main,16},      "3dmap_level16");
    createTablePiece({eCampaignLevelType_Main,17},      "3dmap_level17");
    createTablePiece({eCampaignLevelType_Main,18},      "3dmap_level18");
    createTablePiece({eCampaignLevelType_Main,19},      "3dmap_level19");
    createTablePiece({eCampaignLevelType_Main,20},      "3dmap_level20");
}
