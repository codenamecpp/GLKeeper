#include "pch.h"
#include "MapInteractionController.h"
#include "InputsManager.h"
#include "RenderScene.h"
#include "GraphicsDevice.h"
#include "GameWorld.h"
#include "GameMain.h"

const int MaxTilesSelectionRectWide = 9;

MapInteractionController::MapInteractionController()
{
}

void MapInteractionController::UpdateFrame()
{
    TerrainTile* prevHoveredTile = mHoveredTile;
    ScanHoveredTile();

    if (prevHoveredTile != mHoveredTile)
    {
        OnSelectionChanged();
    }
}

void MapInteractionController::ResetState()
{
    mCurrentMode = eMapInteractionMode_Free;
    mHoveredTile = nullptr;
    mConstructRoomDef = nullptr;
    mConstructTrapDef = nullptr;
    mSelectionStartTile = nullptr;
}

void MapInteractionController::SetRoomsConstruction(RoomDefinition* roomDefinition)
{
    if (roomDefinition == nullptr)
    {
        debug_assert(false);
        return;
    }

    if (mCurrentMode == eMapInteractionMode_ConstructRooms)
    {
        if (mConstructRoomDef == roomDefinition)
            return;
    }

    mCurrentMode = eMapInteractionMode_ConstructRooms;
    EndMultitileSelection(false);
    // setup interaction params
    mConstructRoomDef = roomDefinition;
    OnInteractionModeChanged();
}

void MapInteractionController::SetTrapsConstruction(GameObjectDefinition* trapDefinition)
{
    if (trapDefinition == nullptr)
    {
        debug_assert(false);
        return;
    }

    if (mCurrentMode == eMapInteractionMode_ConstructTraps)
    {
        if (mConstructTrapDef == trapDefinition)
            return;
    }

    mCurrentMode = eMapInteractionMode_ConstructTraps;
    EndMultitileSelection(false);
    // setup interaction params
    mConstructTrapDef = trapDefinition;
    OnInteractionModeChanged();
}

void MapInteractionController::SetFreeModeInteraction()
{
    if (mCurrentMode == eMapInteractionMode_Free)
        return;

    mCurrentMode = eMapInteractionMode_Free;
    EndMultitileSelection(false);
    // setup interaction params
    OnInteractionModeChanged();
}

void MapInteractionController::SetSellRoomsInteraction()
{
    if (mCurrentMode == eMapInteractionMode_SellRooms)
        return;

    mCurrentMode = eMapInteractionMode_SellRooms;
    EndMultitileSelection(false);
    // setup interaction params
    OnInteractionModeChanged();
}

void MapInteractionController::SetDigTerrainInteraction()
{
    if (mCurrentMode == eMapInteractionMode_DigTerrain)
        return;

    mCurrentMode = eMapInteractionMode_DigTerrain;
    EndMultitileSelection(false);
    OnInteractionModeChanged();
}

void MapInteractionController::ProcessInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (inputEvent.mButton == eMouseButton_Left)
    {
        if (inputEvent.mPressed)
        {
            // start multiline selection
            if (CanMultitileSelect())
            {
                BeginMultitileSelection();
                return;
            }

            ProcessSingleTileInteraction();
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

    if (inputEvent.mButton == eMouseButton_Right)
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
            if (mCurrentMode != eMapInteractionMode_DigTerrain)
            {
                SetFreeModeInteraction();
                return;
            }

            ProcessSingleTileInteractionAlt();
        }
        return;
    }
}

void MapInteractionController::ProcessInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void MapInteractionController::ProcessInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void MapInteractionController::ProcessInputEvent(KeyInputEvent& inputEvent)
{
}

void MapInteractionController::ProcessInputEvent(KeyCharEvent& inputEvent)
{
}

void MapInteractionController::ScanHoveredTile()
{
    mHoveredTile = nullptr;

    Point mouseScreenPos = gInputsManager.mCursorPosition;
    cxx::ray3d ray3d;
    if (!gRenderScene.mCamera.CastRayFromScreenPoint(mouseScreenPos, gGraphicsDevice.mViewportRect, ray3d))
        return; // failed

    float distanceNear;
    float distanceFar;

    if (!cxx::intersects(gGameWorld.mMapData.mBounds, ray3d, distanceNear, distanceFar))
        return; // not intersected

    float coordx = ray3d.mOrigin.x + ray3d.mDirection.x * distanceNear;
    float coordz = ray3d.mOrigin.z + ray3d.mDirection.z * distanceNear;
    mHoveredTile = gGameWorld.mMapData.GetTileFromCoord3d(glm::vec3(coordx, 0.0f, coordz));
}

bool MapInteractionController::GetTerrainSelectionArea(Rectangle& selectionArea) const
{
    if (mHoveredTile == nullptr)
        return false;

    if (mSelectionStartTile && mSelectionStartTile != mHoveredTile)
    {
        const int CX = glm::clamp(mSelectionStartTile->mTileLocation.x - mHoveredTile->mTileLocation.x, 
            -MaxTilesSelectionRectWide + 1, MaxTilesSelectionRectWide - 1);

        const int CY = glm::clamp(mSelectionStartTile->mTileLocation.y - mHoveredTile->mTileLocation.y, 
            -MaxTilesSelectionRectWide + 1, MaxTilesSelectionRectWide - 1);

        selectionArea.x = std::min(mSelectionStartTile->mTileLocation.x, mSelectionStartTile->mTileLocation.x - CX);
        selectionArea.y = std::min(mSelectionStartTile->mTileLocation.y, mSelectionStartTile->mTileLocation.y - CY);
        selectionArea.w = std::abs(CX) + 1;
        selectionArea.h = std::abs(CY) + 1;
    }
    else
    {
        selectionArea.x = mHoveredTile->mTileLocation.x;
        selectionArea.y = mHoveredTile->mTileLocation.y;
        selectionArea.w = 1;
        selectionArea.h = 1;
    }
    return true;
}

void MapInteractionController::OnSelectionChanged()
{
    Rectangle selectionArea;
    if (NeedToShowSelection() && GetTerrainSelectionArea(selectionArea))
    {
        gGameWorld.mTerrainCursor.UpdateCursor(selectionArea);
    }
    else
    {
        gGameWorld.mTerrainCursor.ClearCursor();
    }
}

bool MapInteractionController::CanMultitileSelect() const
{
    if (mHoveredTile == nullptr)
        return false;

    const TerrainDefinition* terrainDef = mHoveredTile->GetTerrain();
    switch (mCurrentMode)
    {
    case eMapInteractionMode_Free:
        return terrainDef->mIsTaggable;

    case eMapInteractionMode_ConstructRooms:
    case eMapInteractionMode_SellRooms:
        return true;
    }

    return false;
}

bool MapInteractionController::NeedToShowSelection() const
{
    if (IsMultitileSelectionStarted()) // multitile selection always shown
        return true;

    if (mHoveredTile)
    {
        const TerrainDefinition* terrainDef = mHoveredTile->GetTerrain();
        switch (mCurrentMode)
        {
        case eMapInteractionMode_CastSpells:
        case eMapInteractionMode_Free:
            return terrainDef->mIsTaggable;

        case eMapInteractionMode_DigTerrain:
            return !terrainDef->mIsImpenetrable && !terrainDef->mIsWater && !terrainDef->mIsLava;

        case eMapInteractionMode_ConstructRooms:
        case eMapInteractionMode_SellRooms:
        case eMapInteractionMode_ConstructTraps:
            return true;
        }
    }
    return false;
}

bool MapInteractionController::IsMultitileSelectionStarted() const
{
    return mSelectionStartTile != nullptr;
}

void MapInteractionController::BeginMultitileSelection()
{
    debug_assert(mHoveredTile);
    if (mHoveredTile)
    {
        mSelectionStartTile = mHoveredTile;
        OnSelectionChanged();
    }
}

void MapInteractionController::EndMultitileSelection(bool success)
{
    if (success)
    {
        Rectangle selectionArea;
        if (GetTerrainSelectionArea(selectionArea))
        {
            ProcessInteractionOnArea(selectionArea);
        }
    }
    mSelectionStartTile = nullptr;
    OnSelectionChanged();
}

void MapInteractionController::OnInteractionModeChanged()
{
    debug_assert(gGameMain.IsGameplayGamestate());
    // todo
    gGameMain.mGameplayGamestate.mGameplayGameScreen.UpdateUserInterfaceState();
}

bool MapInteractionController::ProcessTagTerrain(const Rectangle& area)
{
    TerrainDefinition* selectionStartTerrain = mSelectionStartTile ? mSelectionStartTile->GetTerrain() : nullptr;
    if (selectionStartTerrain && selectionStartTerrain->mIsSolid)
    {
        if (mSelectionStartTile->mIsTagged)
        {
            gGameWorld.UnTagTerrain(area);
        }
        else
        {
            gGameWorld.TagTerrain(area);
        }
        return true;
    }
    return false;
}

void MapInteractionController::ProcessInteractionOnArea(const Rectangle& area)
{
    if (mCurrentMode == eMapInteractionMode_Free)
    {
        if (ProcessTagTerrain(area))
            return;

        return;
    }

    if (mCurrentMode == eMapInteractionMode_ConstructRooms)
    {
        if (ProcessTagTerrain(area))
            return;

        gGameWorld.ConstructRoom(ePlayerID_Keeper1, mConstructRoomDef, area);
        return;
    }

    if (mCurrentMode == eMapInteractionMode_SellRooms)
    {
        gGameWorld.SellRooms(ePlayerID_Keeper1, area);
        return;
    }
}

void MapInteractionController::ProcessSingleTileInteraction()
{
    if (mHoveredTile == nullptr)
        return;

    if (mCurrentMode == eMapInteractionMode_DigTerrain)
    {
        // todo
        //gWorldState.RepairTerrainTile(mHoveredTile, ePlayerID_Keeper1, 999999);
        return;
    }
}

void MapInteractionController::ProcessSingleTileInteractionAlt()
{
    if (mHoveredTile == nullptr)
        return;

    if (mCurrentMode == eMapInteractionMode_DigTerrain)
    {
        // todo
        //gWorldState.DamageTerrainTile(mHoveredTile, ePlayerID_Keeper1, 999999);
        return;
    }
}