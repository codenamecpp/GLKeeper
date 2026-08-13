#include "stdafx.h"
#include "Room.h"
#include "TileConstructor.h"
#include "GameMap.h"
#include "MapTile.h"
#include "GameRenderManager.h"
#include "GameObject.h"
#include "MeshAssetManager.h"
#include "GameWorld.h"
#include "SimplePool.h"
#include "GameMain.h"
#include "RoomManager.h"
#include "GameObjectManager.h"

//////////////////////////////////////////////////////////////////////////

static SimplePool<RoomWallSection> gRoomWallSectionsPool;

//////////////////////////////////////////////////////////////////////////

void Room::ConfigureInstance(EntityUid instanceUid, 
    RoomController* controller, RoomTileConstructor* tileConstructor, 
    RoomDefinition* definition, eDirection direction, ePlayerID owner)
{
    mInstanceUid = instanceUid;
    mRoomDirection = direction;

    mOwnerId = owner;
    cxx_assert(mOwnerId != ePlayerID_Null);

    cxx_assert((mDefinition == nullptr) && definition);
    mDefinition = definition;

    cxx_assert((mTileConstructor == nullptr) && controller);
    mTileConstructor = tileConstructor;

    cxx_assert((mController == nullptr) && controller);
    mController = controller;
    mController->ConfigureInstance(this);
}

void Room::SpawnInstance()
{
    cxx_assert(!mLifecycleFlags.mWasSpawned);
    cxx_assert(!mLifecycleFlags.mWasDespawned);
    cxx_assert(!mLifecycleFlags.mWasDeleted);

    if (mLifecycleFlags.mWasSpawned) 
        return;

    mLifecycleFlags.mWasSpawned = true;

    mLocationArea = {};
    mOwnHandle = gRoomManager.FindRoom(mInstanceUid);

    mController->SpawnInstance();
}

void Room::DespawnInstance()
{
    cxx_assert(mLifecycleFlags.mWasSpawned);

    if (mLifecycleFlags.mWasDespawned) return;

    mController->DespawnInstance();

    mOwnerId = ePlayerID_Null;
    mOwnHandle = {};
    mLocationArea = {};
    ReleaseWallSections();
    UnassignTiles(mCoveredTiles);
    DeleteFurnitureObjects();
    mInnerTiles.clear();
    mStorageSlots.clear();

    mLifecycleFlags.mWasDespawned = true;
}

void Room::UpdateLogic(float stepDeltaTime)
{
    if (mController)
    {
        mController->UpdateLogic(stepDeltaTime);
    }
}

void Room::AbsorbRoom(Room* sourceRoom)
{
    cxx_assert(sourceRoom);
    cxx_assert(sourceRoom != this);
    cxx_assert(mDefinition == sourceRoom->mDefinition);
    if (sourceRoom == this)
        return;

    // transfer room objects
    if (!sourceRoom->TransferRoomObjectsTo(this))
    {
        cxx_assert(false);
    }
    auto tilesToAbsorb = TempVectorFrom<MapTile*>(sourceRoom->mCoveredTiles);
    sourceRoom->ReleaseTiles(tilesToAbsorb);
    EnlargeRoom(tilesToAbsorb);
}

void Room::AbsorbRoom(Room* sourceRoom, cxx::span<MapTile*> targetTiles)
{
    cxx_assert(sourceRoom);
    cxx_assert(sourceRoom != this);
    cxx_assert(mDefinition == sourceRoom->mDefinition);
    if (sourceRoom == this)
        return;

    // make sure that target tiles are belongs to source room
    for (MapTile* roller: targetTiles) 
    {  
        cxx_assert(roller->mRoomInstance == sourceRoom);
        if (roller->mRoomInstance != sourceRoom)
            return;
    }

    // transfer room objects
    if (!sourceRoom->TransferRoomObjectsTo(this, targetTiles))
    {
        cxx_assert(false);
    }
    sourceRoom->ReleaseTiles(targetTiles);
    EnlargeRoom(targetTiles);
}

bool Room::CanConstructOn(const MapTile* targetTile)
{
    if (!targetTile || targetTile->mRoomInstance)
        return false;

    // must build room on specific terrain type
    const TerrainDefinition* terrain = targetTile->GetTerrain();
    return (terrain->mTerrainType == mDefinition->mTerrainType);
}

void Room::AssignTiles(cxx::span<MapTile*> targetTiles)
{
    // first scan all good tiles and assign room instance to them
    for (MapTile* targetTile: targetTiles)
    {
        bool canConstructRoom = CanConstructOn(targetTile);
        cxx_assert(canConstructRoom);
        if (!canConstructRoom)
            continue;
        cxx_assert(targetTile->mIsRoomEntrance == false);
        cxx_assert(targetTile->mIsRoomInnerTile == false);
        cxx_assert(targetTile->mRoomInstance == nullptr);
        targetTile->mRoomInstance = this;
#ifdef _DEBUG
        // check no room walls
        if (MapTile* neighTile = targetTile->mNeighbours[eDirection_N]) { cxx_assert(neighTile->mFaces[eTileFace_SideS].mWallExtendsRoom == false); }
        if (MapTile* neighTile = targetTile->mNeighbours[eDirection_E]) { cxx_assert(neighTile->mFaces[eTileFace_SideW].mWallExtendsRoom == false); }
        if (MapTile* neighTile = targetTile->mNeighbours[eDirection_S]) { cxx_assert(neighTile->mFaces[eTileFace_SideN].mWallExtendsRoom == false); }
        if (MapTile* neighTile = targetTile->mNeighbours[eDirection_W]) { cxx_assert(neighTile->mFaces[eTileFace_SideE].mWallExtendsRoom == false); }
#endif
        // invalidate tiles
        gGameWorld.InvalidateTile(targetTile);
        gGameWorld.InvalidateTileNeighbours(targetTile);
        mCoveredTiles.push_back(targetTile);
    }
}

void Room::UnassignTiles(cxx::span<MapTile*> targetTiles)
{
    // unassign removed tiles
    for (MapTile* targetTile: targetTiles)
    {
        if (targetTile->mRoomInstance != this) 
            continue;
        targetTile->mRoomInstance = nullptr;
        targetTile->mIsRoomEntrance = false;
        targetTile->mIsRoomInnerTile = false;
        // remove wall references
        DetachFromWall(targetTile);
        // invalidate tiles
        gGameWorld.InvalidateTile(targetTile);
        gGameWorld.InvalidateTileNeighbours(targetTile);
    }

    // cleanup covered tiles
    cxx::erase_if(mCoveredTiles, [](const MapTile* dungeonMapTile)
        {
            return dungeonMapTile->mRoomInstance == nullptr;
        });
}

void Room::EnlargeRoom(cxx::span<MapTile*> targetTiles)
{
    AssignTiles(targetTiles);
    // update bounds and inner squares
    ReevaluateOccupationArea();
    ReevaluateInnerSquares();
    ReevaluateWallSections();
    Reconfigure();
}

void Room::ReleaseTiles(cxx::span<MapTile*> targetTiles)
{
    UnassignTiles(targetTiles);
    // update bounds and inner squares
    ReevaluateOccupationArea();
    ReevaluateInnerSquares();
    ReevaluateWallSections();
    Reconfigure();
}

void Room::ReleaseTiles()
{
    auto releasedTiles = TempVectorFrom<MapTile*>(mCoveredTiles);
    mCoveredTiles.clear();
    ReleaseTiles(releasedTiles);
}

void Room::PostRearrangeObjects()
{
    GameObjectManager& gobjects = gGameObjectManager;

    // make sure to ground objects
    for (const RoomFurnitureSlot& rollerSlot: mFloorFurniture)
    {
        if (GameObject* furnitureObject = gobjects.GetObjectPtr(rollerSlot.mObjectHandle))
        {
            furnitureObject->SnapPositionToFloor();
        }
    }

    // make sure to ground pillars
    for (const RoomFurnitureSlot& rollerSlot: mPillars)
    {
        if (GameObject* pillarObject = gobjects.GetObjectPtr(rollerSlot.mObjectHandle))
        {
            pillarObject->SnapPositionToFloor(true);
        }
    }

    mController->PostRearrangeObjects();
}

void Room::Reconfigure()
{
    mController->PostReconfigureRoom();
}

void Room::ReevaluateOccupationArea()
{
    // Room is null
    if (mCoveredTiles.empty())
    {
        mLocationArea = {};
        return;
    }

    MapPoint2D rightBottomPoint = mCoveredTiles[0]->mLocation;
    MapPoint2D leftTopPoint = mCoveredTiles[0]->mLocation;

    for (const MapTile* tile : mCoveredTiles)
    {
        if (tile->mLocation.x < leftTopPoint.x) leftTopPoint.x = tile->mLocation.x;
        else rightBottomPoint.x = tile->mLocation.x;

        if (tile->mLocation.y < leftTopPoint.y) leftTopPoint.y = tile->mLocation.y;
        else rightBottomPoint.y = tile->mLocation.y;
    }

    mLocationArea.x = leftTopPoint.x;
    mLocationArea.y = leftTopPoint.y;
    mLocationArea.w = (rightBottomPoint.x > 0) ? (rightBottomPoint.x - leftTopPoint.x + 1) : 1;
    mLocationArea.h = (rightBottomPoint.y > 0) ? (rightBottomPoint.y - leftTopPoint.y + 1) : 1;
}

void Room::ReevaluateWallSections()
{
    if (!mDefinition->mHasWalls) // not all rooms has walls
        return;

    // todo: optimize walls reevaluation
    ReleaseWallSections();

    // scan wall sections
    for (MapTile* roomTile: mCoveredTiles)
    {
        if (roomTile->mIsRoomInnerTile) // keep looking edge tiles
            continue;

        for (eDirection outOfRoomDirection: gStraightDirections)
        {
            MapTile* neighbourTile = roomTile->mNeighbours[outOfRoomDirection];
            cxx_assert(neighbourTile);

            if (neighbourTile == nullptr)
                continue; // something is wrong here - rooms cannot be built on map boundaries

            if (!neighbourTile->IsSolidBlock() || !neighbourTile->IsAllowRoomWalls())
                continue;

            eDirection inwardsDirection = GetOppositeDirection(outOfRoomDirection);
            // get adjacent face
            eTileFace inwardsTileFace = DirectionToTileFace(inwardsDirection);

            TileFaceData& faceData = neighbourTile->mFaces[inwardsTileFace];
            if (faceData.mWallExtendsRoom) 
                continue; // already processed

            // create wall section
            RoomWallSection* wallSection = gRoomWallSectionsPool.Acquire();
            wallSection->Reset();

            ScanWallSection(neighbourTile, inwardsTileFace, wallSection);
            FinalizeWallSection(wallSection);
        }
    }
}

void Room::ScanWallSection(MapTile* mapTile, eDirection faceDirection, RoomWallSection* section) const
{
    section->Configure(faceDirection);
    section->RemoveTiles();
    ScanWallSectionImpl(mapTile, section);
}

void Room::ScanWallSection(MapTile* mapTile, eTileFace faceId, RoomWallSection* section) const
{
    section->Configure(faceId);
    section->RemoveTiles();
    ScanWallSectionImpl(mapTile, section);
}

void Room::ScanWallSectionImpl(MapTile* originTile, RoomWallSection* section) const
{
    cxx_assert(originTile);
    cxx_assert(section);

    // explore directions
    eDirection scanDirectionHead = eDirection_COUNT;
    eDirection scanDirectionTail = eDirection_COUNT;
    switch (section->GetDirection())
    {
        case eDirection_N: 
        case eDirection_S: 
            scanDirectionHead = eDirection_W;
            scanDirectionTail = eDirection_E;
        break;
        case eDirection_E: 
        case eDirection_W: 
            scanDirectionHead = eDirection_N;
            scanDirectionTail = eDirection_S;
        break;
    }

    // push initial tile
    section->AppendHead(originTile);

    auto ScanLine = [originTile, section, this](eDirection scanDirection, bool isHead)
    {
        for (MapTile* currTile = originTile->mNeighbours[scanDirection]; 
            currTile; currTile = currTile->mNeighbours[scanDirection])
        {
            TerrainDefinition* currTerrain = currTile->GetTerrain();
            
            if (!currTerrain->mIsSolid || !currTerrain->mAllowRoomWalls)
                return;

            MapTile* neighbourTile = currTile->mNeighbours[section->GetDirection()];
            cxx_assert(neighbourTile);

            if (neighbourTile->mRoomInstance != this) return;

            if (isHead)
            {
                section->AppendHead(currTile);
            }
            else
            {
                section->AppendTail(currTile);
            }
        }
    };

    ScanLine(scanDirectionHead, true);
    ScanLine(scanDirectionTail, false);
}

void Room::ReleaseWallSections()
{
    for (RoomWallSection* currentSection: mWallSections)
    {
        eTileFace tileFace = currentSection->GetFace();

        for (MapTile* currentTile: currentSection->GetTiles())
        {
            TileFaceData& faceData = currentTile->mFaces[tileFace];
            if (faceData.mWallExtendsRoom)
            {
                faceData.mWallExtendsRoom = false;
                gGameWorld.InvalidateTile(currentTile);
            }
        }
        currentSection->Reset();
        gRoomWallSectionsPool.Return(currentSection);
    }
    mWallSections.clear();
}

void Room::DetachFromWall(MapTile* roomTile)
{
    cxx_assert(roomTile);

    for (eDirection outOfTileDirection: gStraightDirections)
    {
        MapTile* currNeighbour = roomTile->mNeighbours[outOfTileDirection];
        if (currNeighbour == nullptr)
            continue;

        // inward direction
        eDirection inwardsDirection = GetOppositeDirection(outOfTileDirection);
        eTileFace inwardsTileFace = DirectionToTileFace(inwardsDirection);

        if (RoomWallSection* wallSection = FindWallSectionWithTile(currNeighbour, inwardsTileFace))
        {
            TileFaceData& tileFaceData = currNeighbour->mFaces[inwardsTileFace];
            cxx_assert(tileFaceData.mWallExtendsRoom);
            tileFaceData.mWallExtendsRoom = false;
            wallSection->RemoveTile(currNeighbour);
        }
    }
}

void Room::FinalizeWallSection(RoomWallSection* section)
{
    cxx_assert(section);
    mWallSections.push_back(section);

    const eTileFace tileFace = section->GetFace();
    for (MapTile* currTile: section->GetTiles())
    {
        TileFaceData& faceData = currTile->mFaces[tileFace];
        cxx_assert(!faceData.mWallExtendsRoom);
        faceData.mWallExtendsRoom = true;
        gGameWorld.InvalidateTile(currTile);
    }
}

RoomWallSection* Room::FindWallSectionWithTile(MapTile* mapTile, eTileFace face) const
{
    cxx_assert(mapTile);
    for (RoomWallSection* roller: mWallSections)
    {
        if (roller->GetFace() != face) continue;
        if (roller->ContainsTile(mapTile))
            return roller;
    }
    return nullptr;
}

void Room::OnRecycle()
{
    Entity::OnRecycle();
    EnableEntityComponents::OnRecycle();
    EnableEntityCapabilities::OnRecycle();

    mController = nullptr;
    mDefinition = nullptr;
    mTileConstructor = nullptr;
    mRoomDirection = {};
    cxx_assert(mInnerTiles.empty());
    cxx_assert(mWallSections.empty());
    cxx_assert(mFloorFurniture.empty());
    cxx_assert(mWallsFurniture.empty());
    cxx_assert(mPillars.empty());
    cxx_assert(mStorageSlots.empty());
}

void Room::ReevaluateInnerSquares()
{
    auto CheckIsInnerTile = [this](MapTile* mapTile) -> bool
    {
        int numPassed = 0;
        for (eDirection direction: gDirectionsCW)
        {
            bool isSameRoom = mapTile->SameNeighbourRoomInstance(direction);
            if (!isSameRoom)
                break;

            ++numPassed;
        }
        return (numPassed == eDirection_COUNT);
    };

    mInnerTiles.clear();

    for (MapTile* targetTile : mCoveredTiles)
    {
        bool isInnerTile = CheckIsInnerTile(targetTile);
        if (isInnerTile != targetTile->mIsRoomInnerTile)
        {
            // invalidate tiles
            gGameWorld.InvalidateTile(targetTile);
            gGameWorld.InvalidateTileNeighbours(targetTile);
        }
        targetTile->mIsRoomInnerTile = isInnerTile;
        if (isInnerTile)
        {
            mInnerTiles.push_back(targetTile);
        }
    }
}

bool Room::TransferRoomObjectsTo(Room* receiver, cxx::span<MapTile*> targetTiles)
{
    cxx_assert(receiver);
    cxx_assert(receiver != this);
    if ((receiver == this) || (receiver == nullptr)) return false;

    GameObjectManager& gobjects = gGameObjectManager;

    Temp_List<EntityHandle> objectsTransferred;

    // floor furniture
    for (auto roller_it = mFloorFurniture.begin(); roller_it != mFloorFurniture.end(); )
    {
        if (cxx::contains_if(targetTiles, [&roller_it](MapTile* mapTile) { return mapTile->mLocation == roller_it->mTileLocation; }))
        {
            objectsTransferred.push_back(roller_it->mObjectHandle);
            receiver->mFloorFurniture.push_back(*roller_it);
            roller_it = mFloorFurniture.erase(roller_it);

            continue;
        }
        ++roller_it;
    }

    // wall furniture
    for (auto roller_it = mWallsFurniture.begin(); roller_it != mWallsFurniture.end(); )
    {
        if (cxx::contains_if(targetTiles, [&roller_it](MapTile* mapTile) { return mapTile->mLocation == roller_it->mTileLocation; }))
        {
            objectsTransferred.push_back(roller_it->mObjectHandle);
            receiver->mWallsFurniture.push_back(*roller_it);
            roller_it = mWallsFurniture.erase(roller_it);

            continue;
        }
        ++roller_it;
    }

    // pillars
    for (auto roller_it = mPillars.begin(); roller_it != mPillars.end(); )
    {
        if (cxx::contains_if(targetTiles, [&roller_it](MapTile* mapTile) { return mapTile->mLocation == roller_it->mTileLocation; }))
        {
            objectsTransferred.push_back(roller_it->mObjectHandle);
            receiver->mPillars.push_back(*roller_it);
            roller_it = mPillars.erase(roller_it);

            continue;
        }
        ++roller_it;
    }

    // stored objects
    for (auto roller_it = mStorageSlots.begin(); roller_it != mStorageSlots.end(); )
    {
        if (cxx::contains_if(targetTiles, [&roller_it](MapTile* mapTile) { return mapTile->mLocation == roller_it->mTileLocation; }))
        {
            objectsTransferred.push_back(roller_it->mObjectHandle);
            receiver->mStorageSlots.push_back(*roller_it);
            roller_it = mStorageSlots.erase(roller_it);

            continue;
        }
        ++roller_it;
    }

    // notify parent room changed
    if (!objectsTransferred.empty())
    {
        for (EntityHandle objectHandle: objectsTransferred)
        {
            GameObject* objectPtr = gobjects.GetObjectPtr(objectHandle);
            cxx_assert(objectPtr);

            if (objectPtr)
            {
                objectPtr->ParentRoomChanged(receiver->GetOwnHandle());
            }
        }
    }
    return true;
}

bool Room::TransferRoomObjectsTo(Room* receiver)
{
    cxx_assert(receiver);
    cxx_assert(receiver != this);
    if ((receiver == this) || (receiver == nullptr)) return false;

    Temp_List<EntityHandle> objectsTransferred;

    // floor furniture
    if (!mFloorFurniture.empty())
    {
        for (const auto& roller: mFloorFurniture)
        {
            objectsTransferred.push_back(roller.mObjectHandle);
        }
        receiver->mFloorFurniture.insert(receiver->mFloorFurniture.end(), mFloorFurniture.begin(), mFloorFurniture.end());
        mFloorFurniture.clear();
    }

    // wall furniture
    if (!mWallsFurniture.empty())
    {
        for (const auto& roller: mWallsFurniture)
        {
            objectsTransferred.push_back(roller.mObjectHandle);
        }
        receiver->mWallsFurniture.insert(receiver->mWallsFurniture.end(), mWallsFurniture.begin(), mWallsFurniture.end());
        mWallsFurniture.clear();
    }

    // pillars
    if (!mPillars.empty())
    {
        for (const auto& roller: mPillars)
        {
            objectsTransferred.push_back(roller.mObjectHandle);
        }
        receiver->mPillars.insert(receiver->mPillars.end(), mPillars.begin(), mPillars.end());
        mPillars.clear();
    }

    // storage slots
    if (!mStorageSlots.empty())
    {
        for (const auto& roller: mStorageSlots)
        {
            objectsTransferred.push_back(roller.mObjectHandle);
        }
        receiver->mStorageSlots.insert(receiver->mStorageSlots.end(), mStorageSlots.begin(), mStorageSlots.end());
        mStorageSlots.clear();
    }

    // notify parent room changed
    if (!objectsTransferred.empty())
    {
        GameObjectManager& gobjects = gGameObjectManager;
        for (EntityHandle objectHandle: objectsTransferred)
        {
            GameObject* objectPtr = gobjects.GetObjectPtr(objectHandle);
            cxx_assert(objectPtr);

            if (objectPtr)
            {
                objectPtr->ParentRoomChanged(receiver->GetOwnHandle());
            }
        }
    }
    return true;
}

void Room::NeighbourTileChanged(MapTile* targetTile)
{
    cxx_assert(targetTile);
    bool isNeighbour = false;
    for (eDirection direction: gStraightDirections)
    {
        if (targetTile->mNeighbours[direction] && targetTile->mNeighbours[direction]->mRoomInstance == this)
        {
            isNeighbour = true;
            break;
        }
    }
    cxx_assert(isNeighbour);
    if (isNeighbour)
    {
        // todo: optimize
        ReevaluateWallSections();
    }
}

void Room::DeleteFurnitureObjects()
{
    GameObjectManager& gobjects = gGameObjectManager;

    // floor furniture
    for (const auto& roller: mFloorFurniture)
    {
        gobjects.DeleteObject(roller.mObjectHandle);
    }
    mFloorFurniture.clear();

    // wall furniture
    for (const auto& roller: mWallsFurniture)
    {
        gobjects.DeleteObject(roller.mObjectHandle);
    }
    mWallsFurniture.clear();

    // pillars
    for (const auto& roller: mPillars)
    {
        gobjects.DeleteObject(roller.mObjectHandle);
    }
    mPillars.clear();
}

void Room::RearrangeObjects()
{
    mTempFurnitureEvaluation.reserve(32);

    // floor furniture
    {
        mController->EvaluateFloorFurniture(mTempFurnitureEvaluation);
        HandleRoomFurnitureObjects(mTempFurnitureEvaluation, mFloorFurniture);

        cxx_assert(mFloorFurniture.empty());

        mFloorFurniture.assign(mTempFurnitureEvaluation.begin(), mTempFurnitureEvaluation.end());
        mTempFurnitureEvaluation.clear();
    }

    // wall furniture
    {
        mController->EvaluateWallFurniture(mTempFurnitureEvaluation);
        HandleRoomFurnitureObjects(mTempFurnitureEvaluation, mWallsFurniture);
        
        cxx_assert(mWallsFurniture.empty());

        mWallsFurniture.assign(mTempFurnitureEvaluation.begin(), mTempFurnitureEvaluation.end());
        mTempFurnitureEvaluation.clear();
    }

    // pillars
    {
        mController->EvaluatePillars(mTempFurnitureEvaluation);
        HandleRoomFurnitureObjects(mTempFurnitureEvaluation, mPillars);

        cxx_assert(mPillars.empty());

        mPillars.assign(mTempFurnitureEvaluation.begin(), mTempFurnitureEvaluation.end());
        mTempFurnitureEvaluation.clear();
    }

    PostRearrangeObjects();
}

void Room::HandleRoomFurnitureObjects(cxx::span<RoomFurnitureSlot> newObjects, RoomFurnitureSlots& prevObjects)
{
    auto CamReuse = [](const RoomFurnitureSlot& lhs, const RoomFurnitureSlot& rhs)
        {
            return (lhs.mObjectClassId == rhs.mObjectClassId) && (lhs.mObjectPositioning == rhs.mObjectPositioning) &&
                (lhs.mObjectRotation == rhs.mObjectRotation) && (lhs.mTileLocation == rhs.mTileLocation);
        };

    GameObjectManager& gobjects = gGameObjectManager;
    for (RoomFurnitureSlot& currentObject: newObjects)
    {
        // try reuse prev
        auto it = std::find_if(prevObjects.begin(), prevObjects.end(), 
            [&currentObject, &CamReuse](const RoomFurnitureSlot& slot) { return CamReuse(currentObject, slot); });
        if (it != prevObjects.end())
        {
            currentObject.mObjectHandle = it->mObjectHandle;
            prevObjects.erase(it);
            continue;
        }

        // create new object
        currentObject.mObjectHandle = gobjects.CreateObject(currentObject.mObjectClassId);
        if (!currentObject.mObjectHandle)
        {
            cxx_assert(false);
            continue;
        }
        if (!gobjects.ActivateObject(currentObject.mObjectHandle))
        {
            cxx_assert(false);
        }

        GameObject* gameObject = gobjects.GetObjectPtr(currentObject.mObjectHandle);
        cxx_assert(gameObject);

        // setup params
        gameObject->SetMeshResource(currentObject.mObjectMeshId);
        gameObject->ParentRoomChanged(GetOwnHandle());

        // setup object position and rotation
        glm::vec3 gobjectPosition = MapUtils::ComputeTileCenter(currentObject.mTileLocation);

        gameObject->SetPosition(gobjectPosition);
        if (currentObject.mObjectRotation != RoomFurnitureSlot::eFaceRotation_0)
        {
            float andgeDegrees = 0.0f;
            switch (currentObject.mObjectRotation)
            {
                case RoomFurnitureSlot::eFaceRotation_180   : andgeDegrees = 180.0f; break;
                case RoomFurnitureSlot::eFaceRotation_90_Neg: andgeDegrees = -90.0f; break;
                case RoomFurnitureSlot::eFaceRotation_90_Pos: andgeDegrees =  90.0f; break;
                case RoomFurnitureSlot::eFaceRotation_Random:
                {
                    andgeDegrees = Random::GenerateAngleDegrees();
                }
                break;
            }
            gameObject->SetOrientation(cxx::angle_t::from_degrees(andgeDegrees));
        }

        // correct position
        if (currentObject.mObjectPositioning != RoomFurnitureSlot::ePositioning_Default)
        {
            const cxx::aabbox& gobjectBounds = gameObject->GetMeshWorldBounds();
            const cxx::aabbox blockBounds = MapUtils::ComputeBlockBounds(currentObject.mTileLocation);
            switch (currentObject.mObjectPositioning)
            {
                case RoomFurnitureSlot::ePositioning_WallN:
                {
                    float diff = (blockBounds.mMin.z - gobjectBounds.mMin.z);
                    gobjectPosition.z += diff;
                }
                break;
                case RoomFurnitureSlot::ePositioning_WallE:
                {
                    float diff = (blockBounds.mMax.x - gobjectBounds.mMax.x);
                    gobjectPosition.x += diff;
                }
                break;
                case RoomFurnitureSlot::ePositioning_WallS:
                {
                    float diff = (blockBounds.mMax.z - gobjectBounds.mMax.z);
                    gobjectPosition.z += diff;
                }
                break;
                case RoomFurnitureSlot::ePositioning_WallW:
                {
                    float diff = (blockBounds.mMin.x - gobjectBounds.mMin.x);
                    gobjectPosition.x += diff;
                }
                break;
            }
            gameObject->SetPosition(gobjectPosition);
        }
    }

    // destroy unneeded objects
    for (const RoomFurnitureSlot& roller: prevObjects)
    {
        gobjects.DeleteObject(roller.mObjectHandle);
        prevObjects.pop_back();
    }
}

void Room::AssignObjectToStorageSlot(const MapPoint2D& tileLocation, EntityHandle entityHandle)
{
    if (!entityHandle.IsGameObject())
    {
        cxx_assert(false);
        return;
    }

    // make sure that the object has not already been assigned
    cxx_assert(GetStorageSlotIndex(entityHandle) == -1);

    RoomStorageSlot& storageSlot = mStorageSlots.emplace_back();
    storageSlot.mTileLocation = tileLocation;
    storageSlot.mObjectHandle = entityHandle;

    // notify room changed
    GameObject* gameObject = gGameObjectManager.GetObjectPtr(entityHandle);
    cxx_assert(gameObject && !gameObject->GetParentRoom());
    if (gameObject)
    {
        gameObject->ParentRoomChanged(GetOwnHandle());
    }
}

void Room::UnassignStorageSlotObject(const MapPoint2D& tileLocation, EntityHandle entityHandle)
{
    if (!entityHandle.IsGameObject())
    {
        cxx_assert(false);
        return;
    }

    const int slotIndex = GetStorageSlotIndex(entityHandle);
    if (slotIndex == -1)
    {
        cxx_assert(false);
        return;
    }
    mStorageSlots.erase(mStorageSlots.begin() + slotIndex);

    // notify room changed
    GameObject* gameObject = gGameObjectManager.GetObjectPtr(entityHandle);
    if (gameObject && (gameObject->GetParentRoom() == GetOwnHandle()))
    {
        gameObject->ParentRoomChanged({});
    }
    else
    {
        cxx_assert(false);
    }
}

void Room::ReassignStorageSlotObject(const MapPoint2D& tileLocation, EntityHandle entityHandle, const MapPoint2D& newLocation)
{
    cxx_assert(tileLocation != newLocation);
    if (tileLocation == newLocation) return;

    if (!entityHandle.IsGameObject())
    {
        cxx_assert(false);
        return;
    }

    const int slotIndex = GetStorageSlotIndex(entityHandle);
    if (slotIndex == -1)
    {
        cxx_assert(false);
        return;
    }

    mStorageSlots[slotIndex].mTileLocation = newLocation;
}

int Room::GetStorageSlotIndex(EntityHandle entityHandle) const
{
    const int slotIndex = cxx::get_first_index_if(mStorageSlots, [&entityHandle](const RoomStorageSlot& storageSlot)
        { 
            return (storageSlot.mObjectHandle == entityHandle);
        });
    return slotIndex;
}

void Room::MarkDeleted()
{
    mLifecycleFlags.mWasDeleted = true;
}