#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Room.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class RoomController: protected GameSessionAware
{
public:

    //////////////////////////////////////////////////////////////////////////

    // temporary array for room object evaluation

    using FurnitureEvaluationResult = Temp_Vector<RoomFurnitureSlot>;

    //////////////////////////////////////////////////////////////////////////

public:
    virtual ~RoomController();

    // lifecycle

    // performs early initialization of the instance, before it is spawned
    virtual void ConfigureInstance(Room* roomInstance);

    // preloads resources, adds the instance to the game world and activates it
    virtual void SpawnInstance();

    // deactivates the instance and releases its internal resources
    virtual void DespawnInstance();

    // pool
    virtual void OnRecycle();

    // fixed logic tick update
    virtual void UpdateLogic(float stepDeltaTime);

    virtual void EvaluateFloorFurniture(FurnitureEvaluationResult& evaluation);
    virtual void EvaluateWallFurniture(FurnitureEvaluationResult& evaluation);
    virtual void EvaluatePillars(FurnitureEvaluationResult& evaluation);
    virtual void PostRearrangeObjects();
    virtual void PostReconfigureRoom();

protected:
    virtual void ReevaluatePillarTiles();

    // shortcuts
    inline Room& GetRoom() const { return *mRoom; }

    inline GameObjectClassId GetRoomPillarObjectId() const
    {
        RoomDefinition* roomDefinition = GetRoom().GetDefinition();
        return roomDefinition->mPillarObjectId;
    }
protected:
    std::vector<MapTile*> mPillarTiles; // cache

private:
    Room* mRoom = nullptr;
};

//////////////////////////////////////////////////////////////////////////