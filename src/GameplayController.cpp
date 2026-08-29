#include "stdafx.h"
#include "GameplayController.h"
#include "ToolsUiManager.h"
#include "MeshAssetManager.h"
#include "TileConstructor.h"
#include "GameWorld.h"
#include "GameSession.h"
#include "GameEventBus.h"
#include "GameMap.h"
#include "Scene.h"
#include "UiManager.h"
#include "CreatureManager.h"
#include "GameObjectManager.h"
#include "InteractionService.h"

GameplayController::GameplayController()
    : mHUDScreen(*this)
    , mMapInteractionMode(eMapInteractionMode_Free)
    , mConstructRoomDef()
    , mConstructTrapDef()
    , mHoveredTile()
    , mSelectionStartTile()
{
}

void GameplayController::OnSessionLoaded()
{

}

void GameplayController::OnSessionStart()
{
    Player& localPlayer = gGameSession.GetLocalPlayer();

    const Point2D& gameMapDimensions = gGameMap.GetDimensions();
    cxx_assert(gameMapDimensions.x > 0);
    cxx_assert(gameMapDimensions.y > 0);
    glm::vec2 cameraTileCoord = MapUtils::ComputeTileCenter2d(localPlayer.GetStartCameraTilePosition());
    mGameplayCamera.SetStartPosition(cameraTileCoord);
    mGameplayCamera.SetPositionBounds(
        MapUtils::ComputeTileCenter2d({0, 0}),
        MapUtils::ComputeTileCenter2d({gameMapDimensions.x - 1, gameMapDimensions.y - 1}));
    mGameplayCamera.CaptureCamera(&gScene.GetCamera());
    if (!mHUDScreen.IsActive())
    {
        mHUDScreen.Activate();
    }

    gToolsUiManager.RegisterUi(&mDebugToolsUi);
    mDebugToolsUi.SetActive(true);

    // listen game events
    {
        GameEventBus& eventBus = gGameEventBus;
        eventBus.Subscribe(eGameEvent_ResourceAmountChanged, this);
    }
}

void GameplayController::OnSessionShutdown()
{
    mDebugToolsUi.SetActive(false);
    gToolsUiManager.UnregisterUi(&mDebugToolsUi);

    if (mHUDScreen.IsActive())
    {
        mHUDScreen.Deactivate();
        mHUDScreen.Cleanup();
    }
    mHeldEntityView.SetHeldNothing();
    mGameplayCamera.ReleaseCamera();
    mMapInteractionMode = eMapInteractionMode_Free;
    mHoveredTile = nullptr;
    mConstructRoomDef = nullptr;
    mConstructTrapDef = nullptr;
    mSelectionStartTile = nullptr;
    mHoveredEntity = {};
    gGameEventBus.Unsubscribe(this);
}

void GameplayController::UpdateFrame(float deltaTime)
{
    mGameplayCamera.UpdateFrame(deltaTime);

    UpdateHoveredTile();
    UpdateHoveredEntity();
    UpdateHeldThingView();
}

void GameplayController::UpdateLogic(float stepDeltaTime)
{
    UpdateMapSelectionTint();
}

void GameplayController::InputEvent(MouseButtonInputEvent& inputEvent)
{
    mGameplayCamera.InputEvent(inputEvent);

    if (inputEvent.mButton == MBUTTON_LEFT)
    {
        if (inputEvent.mPressed)
        {
            // start multiline selection
            if (CanMultitileSelect())
            {
                BeginMultitileSelection();
                return;
            }

            HandleHoveredEntityInteraction(false);
            HandleSingleTileInteraction(false);
        }
        else // unpress mouse button
        {
            // finish multiline selection
            if (IsMultitileSelectionStarted())
            {
                EndMultitileSelection(true);
                return;
            }
        }
        return;
    }

    if (inputEvent.mButton == MBUTTON_RIGHT)
    {
        if (inputEvent.mPressed)
        {
            // cancel multitile selection
            if (IsMultitileSelectionStarted())
            {
                EndMultitileSelection(false);
                return;
            }

            // cancel current mode
            if (mMapInteractionMode != eMapInteractionMode_Free && mMapInteractionMode != eMapInteractionMode_Dig)
            {
                SetFreeInteraction();
                return;
            }

            HandleHoveredEntityInteraction(true);
            HandleSingleTileInteraction(true);
        }
        return;
    }
}

void GameplayController::InputEvent(KeyInputEvent& inputEvent)
{
    mGameplayCamera.InputEvent(inputEvent);

    if (inputEvent.IsKeyPressed(KEYCODE_ESCAPE))
    {
        gGameEventBus.Send_ReturnToFrontendRequest();
        inputEvent.SetConsumed();
    }
}

void GameplayController::InputEvent(MouseMovedInputEvent& inputEvent)
{
    mGameplayCamera.InputEvent(inputEvent);
}

void GameplayController::InputEvent(MouseScrollInputEvent& inputEvent)
{
    mGameplayCamera.InputEvent(inputEvent);
}

void GameplayController::HandleGameEvent(const GameEvent& eventData)
{
    if (eventData.IsEvent(eGameEvent_ResourceAmountChanged))
    {
        if (eventData.mResourceAmountChanged.mResourceType == eGameResource_Gold)
        {
            mHUDScreen.UpdateMoneyInfo();
        }
        if (eventData.mResourceAmountChanged.mResourceType == eGameResource_Mana)
        {
            mHUDScreen.UpdateManaInfo();
        }
        return;
    }
}

void GameplayController::UpdateHoveredTile()
{
    MapTile* prevHoveredTile = mHoveredTile;
    MapTile* currHoveredTile = ScanHoveredTile();

    if (gUiManager.IsCursorOverUi())
    {
        // special case: 
        // ui consumes mouse release event, preventing it from reaching the gameplay controller
        // therefore we check the mouse button state directly
        if (IsMultitileSelectionStarted() && !gInputs.GetMouseLeftButton())
        {
            EndMultitileSelection(true);
        }

        // reset hovered
        if (!IsMultitileSelectionStarted())
        {
            currHoveredTile = nullptr; 
        }
    }

    if (prevHoveredTile != currHoveredTile)
    {
        mHoveredTile = currHoveredTile;
        OnMapSelectionChanged();
    }
}

void GameplayController::UpdateHoveredEntity()
{
    EntityHandle currHoveredEntity {};

    if (!gUiManager.IsCursorOverUi())
    {
        const Point2D mouseScreenPos = gInputs.GetMousePosition();

        cxx::ray3d_t ray3d;
        if (gScene.CastRayFromScreenPoint(mouseScreenPos, ray3d))
        {
            cxx::temp_vector<SceneObject*> sceneObjects;
            if (gScene.QueryObjects(ray3d, sceneObjects))
            {
                for (SceneObject* objectsRoller: sceneObjects)
                {
                    if (const EntityHandle& entity = objectsRoller->GetOwnerEntity())
                    {
                        cxx_assert(!entity.IsRoom());
                        currHoveredEntity = entity;
                        break;
                    }
                } // for scene objects
            }
        } // if cast ray
    } 

    if (mHoveredEntity != currHoveredEntity)
    {
        EntityHandle prevEntity = mHoveredEntity;
        mHoveredEntity = currHoveredEntity;
        OnHoveredEntityChanged(prevEntity);
    }
}

void GameplayController::OnHoveredEntityChanged(EntityHandle prevEntity)
{
    // todo: refactore

    if (mHoveredEntity)
    {
        if (mHoveredEntity.IsCreature())
        {
            if (Creature* currCreature = gCreatureManager.GetCreaturePtr(mHoveredEntity))
            {
                currCreature->SetHighlighted(true);
            }
        }

        if (mHoveredEntity.IsGameObject())
        {
            if (GameObject* currObject = gGameObjectManager.GetObjectPtr(mHoveredEntity))
            {
                currObject->SetHighlighted(true);
            }
        }
    }

    if (prevEntity)
    {
        if (prevEntity.IsCreature())
        {
            if (Creature* prevCreature = gCreatureManager.GetCreaturePtr(prevEntity))
            {
                prevCreature->SetHighlighted(false);
            }
        }

        if (prevEntity.IsGameObject())
        {
            if (GameObject* currObject = gGameObjectManager.GetObjectPtr(prevEntity))
            {
                currObject->SetHighlighted(false);
            }
        }
    }
}

void GameplayController::HandleHoveredEntityInteraction(bool alt)
{
    // todo: refactore

    if (!mHoveredEntity.IsCreature() || alt)
        return;

    if (mMapInteractionMode == eMapInteractionMode_Free)
    {
        if (gInteractionService.PickUpEntity(mHoveredEntity, gGameSession.GetLocalPlayerId()))
        {
            UpdateHeldThingView();
        }
    }
}

MapTile* GameplayController::ScanHoveredTile() const
{
    const Point2D mouseScreenPos = gInputs.GetMousePosition();
    cxx::ray3d_t ray3d;
    if (!gScene.CastRayFromScreenPoint(mouseScreenPos, ray3d))
        return nullptr; // failed
    
    float distanceNear;
    float distanceFar;

    if (!cxx::intersects(gGameMap.mBoundingBox, ray3d, distanceNear, distanceFar))
        return nullptr; // not intersected
 
    return gGameMap.GetTileAtPosition(ray3d.mOrigin + ray3d.mDirection * distanceNear);
}

bool GameplayController::GetMapSelectionArea(Rect2D& selectionArea) const
{
    if (mHoveredTile == nullptr)
        return false;

    if (mSelectionStartTile && mSelectionStartTile != mHoveredTile)
    {
        const int CX = glm::clamp(mSelectionStartTile->mLocation.x - mHoveredTile->mLocation.x, 
            -MAX_TILE_SELECTION_RECT_WIDE + 1, MAX_TILE_SELECTION_RECT_WIDE - 1);

        const int CY = glm::clamp(mSelectionStartTile->mLocation.y - mHoveredTile->mLocation.y, 
            -MAX_TILE_SELECTION_RECT_WIDE + 1, MAX_TILE_SELECTION_RECT_WIDE - 1);

        selectionArea.x = std::min(mSelectionStartTile->mLocation.x, mSelectionStartTile->mLocation.x - CX);
        selectionArea.y = std::min(mSelectionStartTile->mLocation.y, mSelectionStartTile->mLocation.y - CY);
        selectionArea.w = std::abs(CX) + 1;
        selectionArea.h = std::abs(CY) + 1;
    }
    else
    {
        selectionArea.x = mHoveredTile->mLocation.x;
        selectionArea.y = mHoveredTile->mLocation.y;
        selectionArea.w = 1;
        selectionArea.h = 1;
    }
    return true;
}

void GameplayController::OnMapSelectionChanged()
{
    Rect2D selectionArea;
    if (NeedToShowSelection() && GetMapSelectionArea(selectionArea))
    {
        gGameWorld.GetMapSelectionCursor().UpdateSelection(selectionArea);
        UpdateMapSelectionTint();
    }
    else
    {
        gGameWorld.GetMapSelectionCursor().ClearSelection();
    }
}

bool GameplayController::CanMultitileSelect() const
{
    if (mHoveredTile == nullptr)
        return false;

    const TerrainDefinition* terrainDef = mHoveredTile->GetTerrain();
    switch (mMapInteractionMode)
    {
        case eMapInteractionMode_Free:
            return terrainDef->mIsTaggable;

        case eMapInteractionMode_Build:
        case eMapInteractionMode_Sell:
            return true;
    }

    return false;
}

bool GameplayController::NeedToShowSelection() const
{
    if (IsMultitileSelectionStarted()) // multitile selection always shown
        return true;

    if (mHoveredTile)
    {
        const TerrainDefinition* terrainDef = mHoveredTile->GetTerrain();
        switch (mMapInteractionMode)
        {
            case eMapInteractionMode_CastSpell:
            case eMapInteractionMode_Free:
            {
                if (gGameSession.GetLocalPlayer().HasSomethingInHand())
                    return true;

                return terrainDef->mIsTaggable;
            }

            case eMapInteractionMode_Dig:
            {
                return !terrainDef->mIsImpenetrable && !terrainDef->mIsWater && !terrainDef->mIsLava;
            }

            case eMapInteractionMode_Build:
            case eMapInteractionMode_Sell:
            case eMapInteractionMode_PlaceTrap:
            {
                return true;
            }
        }
    }
    return false;
}

bool GameplayController::IsMultitileSelectionStarted() const
{
    return mSelectionStartTile != nullptr;
}

void GameplayController::BeginMultitileSelection()
{
    cxx_assert(mHoveredTile);
    if (mHoveredTile)
    {
        mSelectionStartTile = mHoveredTile;
        OnMapSelectionChanged();
    }
}

void GameplayController::EndMultitileSelection(bool success)
{
    if (success)
    {
        Rect2D selectionArea;
        if (GetMapSelectionArea(selectionArea))
        {
            HandleInteractionOnArea(selectionArea);
        }
    }
    mSelectionStartTile = nullptr;
    OnMapSelectionChanged();
}

void GameplayController::SetRoomConstructionMode(RoomDefinition* roomDefinition)
{
    cxx_assert(roomDefinition);
    if (!roomDefinition)
        return;

    mMapInteractionMode = eMapInteractionMode_Build;
    EndMultitileSelection(false);
    // setup interaction params
    mConstructRoomDef = roomDefinition;
    OnInteractionModeChanged();
}

void GameplayController::SetTrapConstructionMode(GameObjectDefinition* trapDefinition)
{
    cxx_assert(trapDefinition);
    if (!trapDefinition)
        return;

    mMapInteractionMode = eMapInteractionMode_PlaceTrap;
    EndMultitileSelection(false);
    // setup interaction params
    mConstructTrapDef = trapDefinition;
    OnInteractionModeChanged();
}

void GameplayController::SetRoomSellInteraction()
{
    mMapInteractionMode = eMapInteractionMode_Sell;
    EndMultitileSelection(false);
    // setup interaction params
    OnInteractionModeChanged();
}

void GameplayController::SetDigTerrainInteraction()
{
    mMapInteractionMode = eMapInteractionMode_Dig;
    EndMultitileSelection(false);
    OnInteractionModeChanged();
}

void GameplayController::SetFreeInteraction()
{
    mMapInteractionMode = eMapInteractionMode_Free;
    EndMultitileSelection(false);
    // setup interaction params
    OnInteractionModeChanged();
}

void GameplayController::OnInteractionModeChanged()
{
    mHUDScreen.UpdateHUDState();

    UpdateMapSelectionTint();
}

bool GameplayController::HandleTagForDigging(const Rect2D& tilesArea)
{
    if (mSelectionStartTile && mSelectionStartTile->IsSolidBlock())
    {
        bool wasTagged = mSelectionStartTile->IsTaggedForDigging(gGameSession.GetLocalPlayerId());
        gGameWorld.TagTilesForDigging(tilesArea, gGameSession.GetLocalPlayerId(), !wasTagged);
        return true;
    }
    return false;
}

void GameplayController::UpdateMapSelectionTint()
{
    MapSelectionCursor& mapCursor = gGameWorld.GetMapSelectionCursor();
    if (!mapCursor.HasSelection())
        return;

    MapSelectionCursor::eSelectionTint selectionTint = MapSelectionCursor::eSelectionTint_Blue;
    switch (mMapInteractionMode)
    {
        case eMapInteractionMode_Build:
        {
            selectionTint = MapSelectionCursor::eSelectionTint_Red;

            Rect2D mapArea;
            if (GetMapSelectionArea(mapArea))
            {
                cxx::temp_vector<MapTile*> constructionTiles;
                if (gGameWorld.TestConstructRooms(gGameSession.GetLocalPlayerId(), mConstructRoomDef, mapArea, constructionTiles))
                {
                    selectionTint = MapSelectionCursor::eSelectionTint_Blue;
                }
            }
        }
        break;

        case eMapInteractionMode_Sell:
        {
            selectionTint = MapSelectionCursor::eSelectionTint_Red;

            Rect2D mapArea;
            if (GetMapSelectionArea(mapArea))
            {
                cxx::temp_vector<MapTile*> demolishTiles;
                if (gGameWorld.TestDemolishRooms(gGameSession.GetLocalPlayerId(), mapArea, demolishTiles))
                {
                    selectionTint = MapSelectionCursor::eSelectionTint_Blue;
                }
            }
        }
        break;

        case eMapInteractionMode_Free:
        case eMapInteractionMode_CastSpell:
        case eMapInteractionMode_PlaceTrap:
        case eMapInteractionMode_Dig:
        default:
        break;
    }
    mapCursor.SetSelectionTint(selectionTint);
}

void GameplayController::UpdateHeldThingView()
{
    const bool isCursorOverUi = gUiManager.IsCursorOverUi();

    if (isCursorOverUi && mHeldEntityView.HasHeldThing())
    {
        mHeldEntityView.SetHeldNothing();
    }

    if (!isCursorOverUi)
    {
        mHeldEntityView.SetHeldThing(gGameSession.GetLocalPlayer().GetLastHeldEntity());
        if (mHeldEntityView.HasHeldThing())
        {
            mHeldEntityView.SetScreenPosition(gInputs.GetMousePosition());
        }
    }
}

void GameplayController::HandleInteractionOnArea(const Rect2D& tilesArea)
{
    if (mMapInteractionMode == eMapInteractionMode_Free)
    {
        if (HandleTagForDigging(tilesArea))
            return;

        return;
    }

    if (mMapInteractionMode == eMapInteractionMode_Build)
    {
        if (HandleTagForDigging(tilesArea))
            return;

        cxx::temp_vector<MapTile*> constructionTiles;
        gGameWorld.ConstructRooms(gGameSession.GetLocalPlayerId(), mConstructRoomDef, tilesArea, constructionTiles);
        return;
    }

    if (mMapInteractionMode == eMapInteractionMode_Sell)
    {
        cxx::temp_vector<MapTile*> demolishTiles;
        gGameWorld.DemolishRooms(gGameSession.GetLocalPlayerId(), tilesArea, demolishTiles);
        return;
    }
}

void GameplayController::HandleSingleTileInteraction(bool alt)
{
    if (mHoveredTile == nullptr)
        return;

    if (mMapInteractionMode == eMapInteractionMode_Dig)
    {
        if (alt)
        {
            long goldMined = 0;
            float healthMultiplier = 10000.0f;
            bool success = gGameWorld.DamageTile(mHoveredTile, gGameSession.GetLocalPlayerId(), healthMultiplier) ||
                gGameWorld.DigBlock(mHoveredTile, gGameSession.GetLocalPlayerId(), healthMultiplier) ||
                gGameWorld.MineBlock(mHoveredTile, gGameSession.GetLocalPlayerId(), goldMined, healthMultiplier);
        }

        //gGameWorld.RepairTile(mHoveredTile, gGameSession.GetLocalPlayerId(), 999999);
        return;
    }

    // todo : refactore
    if ((mMapInteractionMode == eMapInteractionMode_Free) && alt)
    {
        // try drop entity
        EntityHandle entityInHand = gGameSession.GetLocalPlayer().GetLastHeldEntity();
        if (entityInHand)
        {
            glm::vec2 dropOnPosition = MapUtils::ComputeTileCenter2d(mHoveredTile->mLocation);
            if (gInteractionService.DropEntityOn(entityInHand, gGameSession.GetLocalPlayerId(), dropOnPosition))
            {
                UpdateHeldThingView();
            }
        }
    }
}