#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObject.h"
#include "ScenarioDefs.h"
#include "GameObjectController.h"
#include "EntityDefs.h"

//////////////////////////////////////////////////////////////////////////

class GameObjectManager final: public cxx::noncopyable
{
private:
    
    //////////////////////////////////////////////////////////////////////////
    struct GameObjectSlot { uint32_t mGeneration = 1; 
        cxx::uniqueptr<GameObject> mObject; 
        cxx::uniqueptr<GameObjectController> mController;
    };
    //////////////////////////////////////////////////////////////////////////

public:

    bool LoadScenario(const ScenarioDefinition& scenarioDef);
    void EnterWorld();
    void ClearWorld();

    void UpdateFrame(float deltaTime);
    void UpdateLogic(float stepDeltaTime);
    void UpdatePhysics(float stepDeltaTime);

    // forcibly refresh active game object lists, destroy objects pending destruction
    // note: this will invalidate list iterators
    void ProcessObjectChanges();

    EntityHandle CreateScenarioObject(const ScenarioObjectThing& objectThing);
    EntityHandle CreateObject(GameObjectClassId classId);
    EntityHandle CreateObject(GameObjectDefinition* classDefinition);
    EntityHandle CreateGoldChest(long startGold, long storageCapacity);
    EntityHandle CreateGoldPile(long goldAmount);

    EntityHandle FindObject(EntityUid objectUid) const;

    GameObject* GetObjectPtr(EntityUid objectUid) const;
    GameObject* GetObjectPtr(const EntityHandle& objectHandle) const;

    bool ActivateObject(const EntityHandle& objectHandle);
    bool ActivateObject(EntityUid objectUid);

    bool DeleteObject(const EntityHandle& objectHandle);
    bool DeleteObject(EntityUid objectUid);

    // check whether the game object was spawned and remains active on map
    bool ExistsOnMap(const EntityHandle& objectHandle) const;

    // get all currently active gameobjects in game world
    // warning: do not store result
    inline cxx::span<GameObject*> GetObjects() const { return mActiveObjects; }
    inline cxx::span<GameObject*> GetObjectsByClass(GameObjectClassId classId) const
    {
        auto map_it = mActiveObjectsByClass.find(classId);
        if (map_it != mActiveObjectsByClass.end())
        {
            return map_it->second;
        }
        return {};
    }
    inline cxx::span<GameObject*> GetObjectsByCategory(eGameObjectCategory objectCategory) const
    {
        auto map_it = mActiveObjectsByCategory.find(objectCategory);
        if (map_it != mActiveObjectsByCategory.end())
        {
            return map_it->second;
        }
        return {};
    }
    // get objects by capabilities
    inline cxx::span<GameObject*> GetGoldContainers() const { return mActiveGoldContainers; }

private:
    // factory
    cxx::uniqueptr<GameObject> NewObjectInstance() const;

    template<typename TController>
    cxx::uniqueptr<GameObjectController> NewControllerInstance() const;
    cxx::uniqueptr<GameObjectController> NewControllerInstance(GameObjectDefinition* objectDefinition) const;

    void ProcessRegistrationQueue();
    void ProcessRemoveQueue();

    void DestroyObjects();

    void ConfigureNewObjectInstance(GameObject* gameObject, 
        GameObjectController* controller, 
        GameObjectDefinition* definition, EntityUid instanceUid);

    void RegisterObject(GameObject* objectInstance);
    void UnregisterObject(GameObject* objectInstance);

private:
    using GameObjectsList = std::vector<GameObject*>;
    std::vector<GameObjectSlot> mObjectSlots;
    std::unordered_map<EntityUid, EntityHandle> mObjectUidsMap;
    std::unordered_map<GameObjectClassId, GameObjectsList> mActiveObjectsByClass;
    std::unordered_map<eGameObjectCategory, GameObjectsList> mActiveObjectsByCategory;
    GameObjectsList mActiveGoldContainers;
    GameObjectsList mActiveObjects;
    std::vector<EntityHandle> mRegistrationQueue; // pending registration in lists
    std::vector<EntityHandle> mRemoveQueue; // pending destroy
};

//////////////////////////////////////////////////////////////////////////

extern GameObjectManager gGameObjectManager;

//////////////////////////////////////////////////////////////////////////