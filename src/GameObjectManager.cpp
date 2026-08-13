#include "stdafx.h"
#include "GameObjectManager.h"
#include "SimplePool.h"
#include "GameWorld.h"
#include "GameObjectController.h"
#include "ChickenObjectController.h"
#include "GoldPileObjectController.h"
#include "MapUtils.h"
#include "GameSession.h"

//////////////////////////////////////////////////////////////////////////

GameObjectManager gGameObjectManager;

//////////////////////////////////////////////////////////////////////////

bool GameObjectManager::LoadScenario(const ScenarioDefinition& scenarioDef)
{
    for (const ScenarioObjectThing& thing: scenarioDef.mObjectThings)
    {
        EntityHandle entHandle = CreateScenarioObject(thing);
        if (entHandle)
        {
            if (!ActivateObject(entHandle))
            {
                cxx_assert(false);
            }
        }
        else
        {
            cxx_assert(false);
        }
    }

    return true;
}

void GameObjectManager::EnterWorld()
{

}

void GameObjectManager::ClearWorld()
{
    DestroyObjects();
}

void GameObjectManager::UpdateFrame(float deltaTime)
{
    ProcessObjectChanges();
}

void GameObjectManager::UpdateLogic(float stepDeltaTime)
{
    // don't use iterators because of new objects that may be added during the update
    for (size_t i = 0, MaxUpdateObjects = mActiveObjects.size(); i < MaxUpdateObjects; ++i)
    {
        GameObject* gameObject = mActiveObjects[i];
        if (!gameObject->WasDeleted())
        {
            gameObject->UpdateLogic(stepDeltaTime);
        }
    }
}

void GameObjectManager::UpdatePhysics(float stepDeltaTime)
{
    // don't use iterators because of new objects that may be added during the update
    for (size_t i = 0, MaxUpdateObjects = mActiveObjects.size(); i < MaxUpdateObjects; ++i)
    {
        GameObject* gameObject = mActiveObjects[i];
        if (!gameObject->WasDeleted())
        {
            gameObject->UpdatePhysics(stepDeltaTime);
        }
    }
}

void GameObjectManager::ProcessObjectChanges()
{
    ProcessRemoveQueue();
    ProcessRegistrationQueue();
}

EntityHandle GameObjectManager::CreateScenarioObject(const ScenarioObjectThing& objectThing)
{
    EntityHandle objectHandle = CreateObject(objectThing.mObjectClassId);
    
    if (GameObject* objectInstance = GetObjectPtr(objectHandle))
    {
        glm::vec3 position = MapUtils::ComputeTileCenter({objectThing.mPositionX, objectThing.mPositionY});
        objectInstance->SetPosition(position);
        objectInstance->SnapPositionToFloor();

        // trigger id
        // todo

        // keeper spell id
        // todo

        // money amount
        if (auto* containerComponent = objectInstance->GetComponent<MoneyComponent>())
        {
            containerComponent->mAmount = objectThing.mMoneyAmount;
        }

        // owner
        // todo
    }

    return objectHandle;
}

EntityHandle GameObjectManager::CreateObject(GameObjectDefinition* classDefinition)
{
    cxx_assert(classDefinition);
    if (classDefinition == nullptr) return {}; // nothing to create

    int freeSlotIndex = cxx::get_first_index_if(mObjectSlots, [](const GameObjectSlot& slot)
        {
            return !slot.mObject;
        });
    if (freeSlotIndex == -1)
    {
        freeSlotIndex = static_cast<int>(mObjectSlots.size());
        mObjectSlots.emplace_back();
    }

    GameObjectSlot& objectSlot = mObjectSlots[freeSlotIndex];
    objectSlot.mObject = NewObjectInstance();
    objectSlot.mController = NewControllerInstance(classDefinition);

    const EntityHandle objectHandle { eEntityType_GameObject, objectSlot.mGeneration, static_cast<uint32_t>(freeSlotIndex) };
    const EntityUid instanceUid = gGameWorld.GenerateEntityUid();
    mObjectUidsMap[instanceUid] = objectHandle;
    ConfigureNewObjectInstance(objectSlot.mObject.get(), objectSlot.mController.get(), classDefinition, instanceUid);
    return objectHandle;
}

EntityHandle GameObjectManager::CreateObject(GameObjectClassId classId)
{
    if (GameObjectDefinition* objectDefinition = gGameSession.GetScenarioDefinition().GetObjectDefinition(classId))
    {
        return CreateObject(objectDefinition);
    }
    gConsole.LogMessage(eLogLevel_Warning, "Cannot create game object with class id '%d'", classId);
    cxx_assert(false);
    return {};
}

EntityHandle GameObjectManager::CreateGoldChest(long startGold, long storageCapacity)
{
    EntityHandle entityHandle = CreateObject(GameObjectClassId_GoldChest);
    if (GameObject* objectInstance = GetObjectPtr(entityHandle))
    {
        if (auto* containerComponent = objectInstance->GetComponent<MoneyComponent>())
        {
            cxx_assert(startGold > 0);
            containerComponent->mAmount = std::max(startGold, 0L);
            cxx_assert(storageCapacity >= 0);
            containerComponent->mCapacity = std::max(storageCapacity, 0L);
        }
    }
    return entityHandle;
}

EntityHandle GameObjectManager::CreateGoldPile(long goldAmount)
{
    EntityHandle entityHandle = CreateObject(GameObjectClassId_GoldPile);
    if (GameObject* objectInstance = GetObjectPtr(entityHandle))
    {
        if (auto* containerComponent = objectInstance->GetComponent<MoneyComponent>())
        {
            containerComponent->mAmount = goldAmount;
            containerComponent->mCapacity = 0;
        }
    }
    return entityHandle;
}

EntityHandle GameObjectManager::FindObject(EntityUid objectUid) const
{
    auto map_it = mObjectUidsMap.find(objectUid);
    if (map_it != mObjectUidsMap.end())
    {
        return map_it->second;
    }
    return {};
}

GameObject* GameObjectManager::GetObjectPtr(EntityUid objectUid) const
{
    EntityHandle objectHandle = FindObject(objectUid);
    return GetObjectPtr(objectHandle);
}

GameObject* GameObjectManager::GetObjectPtr(const EntityHandle& objectHandle) const
{
    GameObject* gameObjectPtr = nullptr;
    if (objectHandle.IsGameObject() && (objectHandle.mIndex < mObjectSlots.size()))
    {
        const GameObjectSlot& objectSlot = mObjectSlots[objectHandle.mIndex];
        if (objectHandle.mGeneration == objectSlot.mGeneration)
        {
            gameObjectPtr = objectSlot.mObject.get();
        }
    }
    return gameObjectPtr;
}

bool GameObjectManager::ActivateObject(const EntityHandle& objectHandle)
{
    bool isSuccess = false;
    if (GameObject* objectInstance = GetObjectPtr(objectHandle))
    {
        // pending deletion?
        if (cxx::contains(mRemoveQueue, objectHandle))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Attempting to activate deleted game object");
        }
        // already active?
        else if (cxx::contains(mActiveObjects, objectInstance) || cxx::contains(mRegistrationQueue, objectHandle))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Attempting to activate already activated game object");
        }
        else // success
        {
            // register later
            mRegistrationQueue.push_back(objectHandle);
            // handle spawn immediately
            objectInstance->SpawnInstance();
            isSuccess = true;
        }
    }
    else
    {
        gConsole.LogMessage(eLogLevel_Warning, "Attempting to activate non exising game object");
    }
    cxx_assert(isSuccess);
    return isSuccess;
}

bool GameObjectManager::ActivateObject(EntityUid objectUid)
{
    EntityHandle objectHandle = FindObject(objectUid);
    return ActivateObject(objectHandle);
}

bool GameObjectManager::DeleteObject(const EntityHandle& objectHandle)
{
    bool isSuccess = false;

    if (cxx::contains(mRemoveQueue, objectHandle))
    {
        isSuccess = true;
    }
    else
    {
        // make sure to discard pending degistration
        cxx::erase(mRegistrationQueue, objectHandle);

        if (GameObject* gameObjectPtr = GetObjectPtr(objectHandle))
        {
            gameObjectPtr->MarkDeleted();

            mRemoveQueue.push_back(objectHandle);
            isSuccess = true;
        }
        else
        {
            gConsole.LogMessage(eLogLevel_Warning, "Attempting to delete non exising game object");
        }
    }
    cxx_assert(isSuccess);
    return isSuccess;
}

bool GameObjectManager::DeleteObject(EntityUid objectUid)
{
    EntityHandle objectHandle = FindObject(objectUid);
    return DeleteObject(objectHandle);
}

bool GameObjectManager::IsObjectActive(const EntityHandle& objectHandle) const
{
    if (GameObject* objectInstance = GetObjectPtr(objectHandle))
    {
        const EntityLifecycleFlags& lifecycleFlags = objectInstance->GetLifecycleFlags();
        return lifecycleFlags.mWasSpawned && !lifecycleFlags.mWasDeleted && !lifecycleFlags.mWasDespawned;
    }
    return false;
}

cxx::uniqueptr<GameObject> GameObjectManager::NewObjectInstance() const
{
    static SimplePool<GameObject> gameObjectsPool = (
        [](GameObject* object)
        {
            object->DespawnInstance();
            object->OnRecycle();
        });

    GameObject* gameObjectPtr = gameObjectsPool.Acquire();
    return std::move(cxx::uniqueptr<GameObject> (gameObjectPtr, [](GameObject* object)
    {
        if (object)
        {
            gameObjectsPool.Return(object);
        }
    }));
}

template<typename TController>
cxx::uniqueptr<GameObjectController> GameObjectManager::NewControllerInstance() const
{
    static SimplePool<TController> controllersPool = {
        [](TController* object)
        {
            object->DespawnInstance();
            object->OnRecycle();
        }};

    GameObjectController* controllerInstance = controllersPool.Acquire();
    cxx_assert(controllerInstance);

    return std::move(cxx::uniqueptr<GameObjectController> (controllerInstance, [](GameObjectController* object)
    {
        if (object)
        {
            TController* speciticControllerType = static_cast<TController*>(object);
            controllersPool.Return(speciticControllerType);
        }
    }));
}

cxx::uniqueptr<GameObjectController> GameObjectManager::NewControllerInstance(GameObjectDefinition* objectDefinition) const
{
    cxx_assert(objectDefinition);

    if (objectDefinition->mObjectCategory == eGameObjectCategory_Gold)
    {
        return NewControllerInstance<GoldPileObjectController>();
    }

    if ((objectDefinition->mObjectClass == GameObjectClassId_Chicken) || 
        (objectDefinition->mObjectClass == GameObjectClassId_Egg))
    {
        return NewControllerInstance<ChickenObjectController>();
    }

    // default is none
    return nullptr;
}

void GameObjectManager::ProcessRegistrationQueue()
{
    while (!mRegistrationQueue.empty())
    {
        const EntityHandle objectHandle = mRegistrationQueue.back();

        // get slot
        if (objectHandle.IsGameObject() && (objectHandle.mIndex < mObjectSlots.size()))
        {
            GameObjectSlot& objectSlot = mObjectSlots[objectHandle.mIndex];
            if (objectHandle.mGeneration == objectSlot.mGeneration)
            {
                GameObject* objectInstance = objectSlot.mObject.get();
                RegisterObject(objectInstance);
            }
            else
            {
                cxx_assert(false);
            }
        }

        mRegistrationQueue.pop_back();
    }
}

void GameObjectManager::ProcessRemoveQueue()
{
    while (!mRemoveQueue.empty())
    {
        const EntityHandle objectHandle = mRemoveQueue.back();

        // get slot
        if (objectHandle.IsGameObject() && (objectHandle.mIndex < mObjectSlots.size()))
        {
            GameObjectSlot& objectSlot = mObjectSlots[objectHandle.mIndex];
            if (objectHandle.mGeneration == objectSlot.mGeneration)
            {
                GameObject* objectInstance = objectSlot.mObject.get();
                objectInstance->DespawnInstance();
                UnregisterObject(objectInstance);
                // update slot
                ++objectSlot.mGeneration;
                cxx_assert(objectSlot.mGeneration != 0);
                // release object
                objectSlot.mObject.reset();
                objectSlot.mController.reset();
            }
            else
            {
                cxx_assert(false);
            }
        }
        mRemoveQueue.pop_back();
    }
}

void GameObjectManager::DestroyObjects()
{
    mRegistrationQueue.clear();
    mRemoveQueue.clear();
    mActiveObjects.clear();
    mActiveObjectsByClass.clear();
    mActiveObjectsByCategory.clear();
    mActiveGoldContainers.clear();
    mObjectSlots.clear();
    mObjectUidsMap.clear();
}

void GameObjectManager::ConfigureNewObjectInstance(GameObject* gameObject, 
    GameObjectController* controller, 
    GameObjectDefinition* definition, EntityUid instanceUid)
{
    cxx_assert(gameObject);

    // setup components
    if (definition->mObjectCategory == eGameObjectCategory_Gold)
    {
        gameObject->AddComponent<MoneyComponent>();
    }

    gameObject->ConfigureInstance(instanceUid, controller, definition);
}

void GameObjectManager::RegisterObject(GameObject* objectInstance)
{
    cxx_assert(objectInstance);

    mActiveObjects.push_back(objectInstance);

    const GameObjectClassId objectClassId = objectInstance->GetClassId();
    mActiveObjectsByClass[objectClassId].push_back(objectInstance);

    const eGameObjectCategory objectCategory = objectInstance->GetCategory();
    mActiveObjectsByCategory[objectCategory].push_back(objectInstance);

    if (objectInstance->HasCapability<GoldContainerCapability>())
    {
        mActiveGoldContainers.push_back(objectInstance);
    }
}

void GameObjectManager::UnregisterObject(GameObject* objectInstance)
{
    cxx_assert(objectInstance);

    cxx::erase(mActiveObjects, objectInstance);
    cxx::erase(mActiveGoldContainers, objectInstance);

    mObjectUidsMap.erase(objectInstance->GetInstanceUid());

    const GameObjectClassId objectClassId = objectInstance->GetClassId();
    cxx::erase(mActiveObjectsByClass[objectClassId], objectInstance);

    const eGameObjectCategory objectCategory = objectInstance->GetCategory();
    cxx::erase(mActiveObjectsByCategory[objectCategory], objectInstance);
}
