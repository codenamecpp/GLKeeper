#include "stdafx.h"
#include "GameplayController.h"
#include "ToolsUiManager.h"
#include "MeshAssetManager.h"
#include "TileConstructor.h"
#include "GameSessionAware.h"
#include "GameWorld.h"
#include "GameSession.h"
#include "GameEventBus.h"

GameplayController::GameplayController()
    : mHUDScreen(*this)
    , mInteraction(eMapInteractionMode_Free)
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
    Player& localPlayer = GetGameSession().GetLocalPlayer();

    glm::vec3 cameraTileCoord = MapUtils::ComputeTileCenter(localPlayer.GetStartCameraTilePosition());
    cameraTileCoord[1] = 8.0f; // height
    mWorldViewCamera.SetStartPosition(cameraTileCoord);
    mWorldViewCamera.CaptureCamera(&GetGameWorld().GetMainCamera());
    if (!mHUDScreen.IsActive())
    {
        mHUDScreen.Activate();
    }

    gToolsUiManager.RegisterUi(&mDebugToolsUi);
    mDebugToolsUi.SetActive(true);

    // listen game events
    {
        GameEventBus& eventBus = GetGameEventBus();
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

    mWorldViewCamera.ReleaseCamera();
    mInteraction = eMapInteractionMode_Free;
    mHoveredTile = nullptr;
    mConstructRoomDef = nullptr;
    mConstructTrapDef = nullptr;
    mSelectionStartTile = nullptr;

    GetGameEventBus().Unsubscribe(this);
}

void GameplayController::UpdateFrame(float deltaTime)
{
    mWorldViewCamera.UpdateFrame(deltaTime);

    MapTile* prevHoveredTile = mHoveredTile;
    ScanHoveredTile();

    if (prevHoveredTile != mHoveredTile)
    {
        OnSelectionChanged();
    }
}

void GameplayController::UpdateLogic(float stepDeltaTime)
{

}

void GameplayController::InputEvent(MouseButtonInputEvent& inputEvent)
{
    mWorldViewCamera.InputEvent(inputEvent);

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

            HandleSingleTileInteraction();
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
            if (mInteraction != eMapInteractionMode_Free && mInteraction != eMapInteractionMode_Dig)
            {
                SetFreeInteraction();
                return;
            }

            HandleSingleTileInteractionAlt();
        }
        return;
    }
}

void GameplayController::InputEvent(KeyInputEvent& inputEvent)
{
    mWorldViewCamera.InputEvent(inputEvent);

    if (inputEvent.IsKeyPressed(KEYCODE_ESCAPE))
    {
        GetGameEventBus().Send_ReturnToFrontendRequest();
        inputEvent.SetConsumed();
    }
}

void GameplayController::InputEvent(MouseMovedInputEvent& inputEvent)
{
    mWorldViewCamera.InputEvent(inputEvent);
}

void GameplayController::InputEvent(MouseScrollInputEvent& inputEvent)
{
    mWorldViewCamera.InputEvent(inputEvent);
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

void GameplayController::ScanHoveredTile()
{
    mHoveredTile = nullptr;

    Point2D mouseScreenPos { gInputs.mCursorPositionX, gInputs.mCursorPositionY };
    cxx::ray3d_t ray3d;
    if (!GetGameWorld().CastRayFromScreenPoint(mouseScreenPos, ray3d))
        return; // failed
    
    float distanceNear;
    float distanceFar;

    if (!cxx::intersects(GetGameWorld().GetGameMap().mBoundingBox, ray3d, distanceNear, distanceFar))
        return; // not intersected
 
    mHoveredTile = GetGameWorld().GetGameMap().GetTileAtPosition(ray3d.mOrigin + ray3d.mDirection * distanceNear);
}

bool GameplayController::GetTerrainSelectionArea(MapArea2D* selectionArea) const
{
    if (!mHoveredTile)
        return false;

    if (mSelectionStartTile && mSelectionStartTile != mHoveredTile)
    {
        const int CX = glm::clamp(mSelectionStartTile->mTileLocation.x - mHoveredTile->mTileLocation.x, 
            -MAX_TILE_SELECTION_RECT_WIDE + 1, MAX_TILE_SELECTION_RECT_WIDE - 1);

        const int CY = glm::clamp(mSelectionStartTile->mTileLocation.y - mHoveredTile->mTileLocation.y, 
            -MAX_TILE_SELECTION_RECT_WIDE + 1, MAX_TILE_SELECTION_RECT_WIDE - 1);

        selectionArea->x = std::min(mSelectionStartTile->mTileLocation.x, mSelectionStartTile->mTileLocation.x - CX);
        selectionArea->y = std::min(mSelectionStartTile->mTileLocation.y, mSelectionStartTile->mTileLocation.y - CY);
        selectionArea->w = std::abs(CX) + 1;
        selectionArea->h = std::abs(CY) + 1;
    }
    else
    {
        selectionArea->x = mHoveredTile->mTileLocation.x;
        selectionArea->y = mHoveredTile->mTileLocation.y;
        selectionArea->w = 1;
        selectionArea->h = 1;
    }
    return true;
}

void GameplayController::OnSelectionChanged()
{
    MapArea2D selectionArea;
    if (NeedToShowSelection() && GetTerrainSelectionArea(&selectionArea))
    {
        GetGameWorld().GetTileSelectionOutline().UpdateSelection(selectionArea);
    }
    else
    {
        GetGameWorld().GetTileSelectionOutline().ClearSelection();
    }
}

bool GameplayController::CanMultitileSelect() const
{
    if (mHoveredTile == nullptr)
        return false;

    const TerrainDefinition* terrainDef = mHoveredTile->GetTerrain();
    switch (mInteraction)
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
        switch (mInteraction)
        {
            case eMapInteractionMode_CastSpell:
            case eMapInteractionMode_Free:
                return terrainDef->mIsTaggable;

            case eMapInteractionMode_Dig:
                return !terrainDef->mIsImpenetrable && !terrainDef->mIsWater && !terrainDef->mIsLava;

            case eMapInteractionMode_Build:
            case eMapInteractionMode_Sell:
            case eMapInteractionMode_PlaceTrap:
                return true;
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
        OnSelectionChanged();
    }
}

void GameplayController::EndMultitileSelection(bool success)
{
    if (success)
    {
        MapArea2D selectionArea;
        if (GetTerrainSelectionArea(&selectionArea))
        {
            HandleInteractionOnArea(selectionArea);
        }
    }
    mSelectionStartTile = nullptr;
    OnSelectionChanged();
}

void GameplayController::SetRoomConstruction(RoomDefinition* roomDefinition)
{
    cxx_assert(roomDefinition);
    if (!roomDefinition)
        return;

    mInteraction = eMapInteractionMode_Build;
    EndMultitileSelection(false);
    // setup interaction params
    mConstructRoomDef = roomDefinition;
    OnInteractionModeChanged();
}

void GameplayController::SetTrapConstruction(GameObjectDefinition* trapDefinition)
{
    cxx_assert(trapDefinition);
    if (!trapDefinition)
        return;

    mInteraction = eMapInteractionMode_PlaceTrap;
    EndMultitileSelection(false);
    // setup interaction params
    mConstructTrapDef = trapDefinition;
    OnInteractionModeChanged();
}

void GameplayController::SetRoomSellInteraction()
{
    mInteraction = eMapInteractionMode_Sell;
    EndMultitileSelection(false);
    // setup interaction params
    OnInteractionModeChanged();
}

void GameplayController::SetDigTerrainInteraction()
{
    mInteraction = eMapInteractionMode_Dig;
    EndMultitileSelection(false);
    OnInteractionModeChanged();
}

void GameplayController::SetFreeInteraction()
{
    mInteraction = eMapInteractionMode_Free;
    EndMultitileSelection(false);
    // setup interaction params
    OnInteractionModeChanged();
}

void GameplayController::OnInteractionModeChanged()
{
    mHUDScreen.UpdateHUDState();

    GetGameWorld().GetTileSelectionOutline().SetSelectionTint((mInteraction == eMapInteractionMode_Sell) ?
        TileSelectionOutline::SelectionTint::Dangerous : 
        TileSelectionOutline::SelectionTint::Neutral);
}

bool GameplayController::HandleTagTerrain(const MapArea2D& tilesArea)
{
    if (mSelectionStartTile && mSelectionStartTile->IsTerrainSolid())
    {
        if (mSelectionStartTile->mIsTagged)
        {
            GetGameWorld().UnTagTerrain(tilesArea);
        }
        else
        {
            GetGameWorld().TagTerrain(tilesArea);
        }
        return true;
    }
    return false;
}

void GameplayController::HandleInteractionOnArea(const MapArea2D& tilesArea)
{
    if (mInteraction == eMapInteractionMode_Free)
    {
        if (HandleTagTerrain(tilesArea))
            return;

        return;
    }

    if (mInteraction == eMapInteractionMode_Build)
    {
        if (HandleTagTerrain(tilesArea))
            return;

        GetGameWorld().ConstructRoom(ePlayerID_Keeper1, mConstructRoomDef, tilesArea);
        return;
    }

    if (mInteraction == eMapInteractionMode_Sell)
    {
        GetGameWorld().SellEntities(ePlayerID_Keeper1, tilesArea);
        return;
    }
}

void GameplayController::HandleSingleTileInteraction()
{
    if (mHoveredTile == nullptr)
        return;

    if (mInteraction == eMapInteractionMode_Dig)
    {
        GetGameWorld().RepairTile(mHoveredTile, ePlayerID_Keeper1, 999999);
        return;
    }
}

void GameplayController::HandleSingleTileInteractionAlt()
{
    if (mHoveredTile == nullptr)
        return;

    if (mInteraction == eMapInteractionMode_Dig)
    {
        GetGameWorld().DamageTile(mHoveredTile, ePlayerID_Keeper1, 999999);
        return;
    }
}