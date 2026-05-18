#include "stdafx.h"
#include "CreatureManager.h"
#include "CreatureController.h"
#include "GameWorld.h"
#include "SimplePool.h"

bool CreatureManager::LoadScenario(const ScenarioDefinition& scenarioDef)
{
    for (const ScenarioCreatureThing& thing: scenarioDef.mCreatureThings)
    {
        EntityHandle entHandle = CreateScenarioCreature(thing);
        if (entHandle)
        {
            if (!ActivateCreature(entHandle))
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

void CreatureManager::EnterWorld()
{

}

void CreatureManager::ClearWorld()
{
    DestroyCreatures();
}

void CreatureManager::UpdateFrame(float deltaTime)
{
    ProcessCreatureChanges();

    // don't use iterators because of new creatures that may be added during the update
    for (size_t i = 0, MaxUpdateCreatures = mActiveCreatures.size(); i < MaxUpdateCreatures; ++i)
    {
        Creature* creature = mActiveCreatures[i];
        if (!creature->WasDeleted())
        {
            creature->UpdateFrame(deltaTime);
        }
    }
}

void CreatureManager::UpdateLogic(float stepDeltaTime)
{
    // don't use iterators because of new creatures that may be added during the update
    for (size_t i = 0, MaxUpdateCreatures = mActiveCreatures.size(); i < MaxUpdateCreatures; ++i)
    {
        Creature* creature = mActiveCreatures[i];
        if (!creature->WasDeleted())
        {
            creature->UpdateLogic(stepDeltaTime);
        }
    }
}

void CreatureManager::UpdatePhysics(float stepDeltaTime)
{
    // don't use iterators because of new creatures that may be added during the update
    for (size_t i = 0, MaxUpdateCreatures = mActiveCreatures.size(); i < MaxUpdateCreatures; ++i)
    {
        Creature* creature = mActiveCreatures[i];
        if (!creature->WasDeleted())
        {
            creature->UpdatePhysics(stepDeltaTime);
        }
    }
}

void CreatureManager::ProcessCreatureChanges()
{
    ProcessRemoveQueue();
    ProcessRegistrationQueue();
}

EntityHandle CreatureManager::CreateScenarioCreature(const ScenarioCreatureThing& creatureThing)
{
    EntityHandle creatureHandle = CreateCreature(creatureThing.mCreatureTypeId, creatureThing.mPlayerId);

    if (Creature* creatureInstance = GetCreaturePtr(creatureHandle))
    {
        glm::vec3 position = MapUtils::ComputeTileCenter({creatureThing.mPositionX, creatureThing.mPositionY});
        creatureInstance->SetPosition(position);
        creatureInstance->SetOrientation(Random::GenerateAngle());
        creatureInstance->SnapPositionToFloor();
    }

    return creatureHandle;
}

EntityHandle CreatureManager::CreateCreature(CreatureTypeId creatureTypeId, ePlayerID ownerID)
{
    if (CreatureDefinition* definition = GetScenarioDefinition().GetCreatureDefinition(creatureTypeId))
    {
        return CreateCreature(definition, ownerID);
    }
    gConsole.LogMessage(eLogLevel_Warning, "Cannot create creature with type id '%d'", creatureTypeId);
    cxx_assert(false);
    return {};
}

EntityHandle CreatureManager::CreateCreature(CreatureDefinition* definition, ePlayerID ownerID)
{
    cxx_assert(definition);
    if (definition == nullptr) return {}; // nothing to create

    int freeSlotIndex = cxx::get_first_index_if(mCreatureSlots, [](const CreatureSlot& slot)
        {
            return !slot.mCreature;
        });
    if (freeSlotIndex == -1)
    {
        freeSlotIndex = static_cast<int>(mCreatureSlots.size());
        mCreatureSlots.emplace_back();
    }

    CreatureSlot& creatureSlot = mCreatureSlots[freeSlotIndex];
    creatureSlot.mCreature = NewCreatureInstance();
    creatureSlot.mController = NewControllerInstance(definition);

    const EntityHandle creatureHandle { eEntityType_Creature, creatureSlot.mGeneration, static_cast<uint32_t>(freeSlotIndex) };
    const EntityUid instanceUid = GetGameWorld().GenerateEntityUid();
    mCreatureUidsMap[instanceUid] = creatureHandle;
    ConfigureNewCreatureInstance(creatureSlot.mCreature.get(), creatureSlot.mController.get(), definition, instanceUid, ownerID);
    return creatureHandle;
}

EntityHandle CreatureManager::FindCreature(EntityUid creatureUid) const
{
    auto map_it = mCreatureUidsMap.find(creatureUid);
    if (map_it != mCreatureUidsMap.end())
    {
        return map_it->second;
    }
    return {};
}

Creature* CreatureManager::GetCreaturePtr(EntityUid creatureUid) const
{
    EntityHandle creatureHandle = FindCreature(creatureUid);
    return GetCreaturePtr(creatureHandle);
}

Creature* CreatureManager::GetCreaturePtr(const EntityHandle& creatureHandle) const
{
    Creature* creatureInstance = nullptr;
    if (creatureHandle.IsCreature() && (creatureHandle.mIndex < mCreatureSlots.size()))
    {
        const CreatureSlot& creatureSlot = mCreatureSlots[creatureHandle.mIndex];
        if (creatureHandle.mGeneration == creatureSlot.mGeneration)
        {
            creatureInstance = creatureSlot.mCreature.get();
        }
    }
    return creatureInstance;
}

bool CreatureManager::ActivateCreature(const EntityHandle& creatureHandle)
{
    bool isSuccess = false;
    if (Creature* creatureInstance = GetCreaturePtr(creatureHandle))
    {
        // pending deletion?
        if (cxx::contains(mRemoveQueue, creatureHandle))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Attempting to activate deleted creature");
        }
        // already active?
        else if (cxx::contains(mActiveCreatures, creatureInstance) || cxx::contains(mRegistrationQueue, creatureHandle))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Attempting to activate already activated creature");
        }
        else // success
        {
            // register later
            mRegistrationQueue.push_back(creatureHandle);
            // handle spawn immediately
            creatureInstance->SpawnInstance();
            isSuccess = true;
        }
    }
    else
    {
        gConsole.LogMessage(eLogLevel_Warning, "Attempting to activate non exising creature");
    }
    cxx_assert(isSuccess);
    return isSuccess;
}

bool CreatureManager::ActivateCreature(EntityUid creatureUid)
{
    EntityHandle creatureHandle = FindCreature(creatureUid);
    return ActivateCreature(creatureHandle);
}

bool CreatureManager::DeleteCreature(const EntityHandle& creatureHandle)
{
    bool isSuccess = false;

    if (cxx::contains(mRemoveQueue, creatureHandle))
    {
        isSuccess = true;
    }
    else
    {
        // make sure to discard pending degistration
        cxx::erase(mRegistrationQueue, creatureHandle);

        if (Creature* creatureInstance = GetCreaturePtr(creatureHandle))
        {
            creatureInstance->MarkDeleted();

            mRemoveQueue.push_back(creatureHandle);
            isSuccess = true;
        }
        else
        {
            gConsole.LogMessage(eLogLevel_Warning, "Attempting to delete non exising creature");
        }
    }
    cxx_assert(isSuccess);
    return isSuccess;
}

bool CreatureManager::DeleteCreature(EntityUid creatureUid)
{
    EntityHandle creatureHandle = FindCreature(creatureUid);
    return DeleteCreature(creatureHandle);
}

bool CreatureManager::IsCreatureActive(const EntityHandle& creatureHandle) const
{
    if (Creature* creature = GetCreaturePtr(creatureHandle))
    {
        const EntityLifecycleFlags& lifecycleFlags = creature->GetLifecycleFlags();
        return lifecycleFlags.mWasSpawned && !lifecycleFlags.mWasDeleted && !lifecycleFlags.mWasDespawned;
    }
    return false;
}

cxx::uniqueptr<Creature> CreatureManager::NewCreatureInstance() const
{
    static SimplePool<Creature> creaturesPool = (
        [](Creature* creature)
        {
            creature->DespawnInstance();
            creature->OnRecycle();
        });

    Creature* creatureInstance = creaturesPool.Acquire();
    return std::move(cxx::uniqueptr<Creature> (creatureInstance, [](Creature* creature)
        {
            if (creature)
            {
                creaturesPool.Return(creature);
            }
        }));
}

template<typename TController>
cxx::uniqueptr<CreatureController> CreatureManager::NewControllerInstance() const
{
    static SimplePool<TController> controllersPool = {
        [](TController* instance)
        {
            instance->DespawnInstance();
            instance->OnRecycle();
        }};

    CreatureController* controllerInstance = controllersPool.Acquire();
    cxx_assert(controllerInstance);

    return std::move(cxx::uniqueptr<CreatureController> (controllerInstance, [](CreatureController* instance)
    {
        if (instance)
        {
            TController* speciticControllerType = static_cast<TController*>(instance);
            controllersPool.Return(speciticControllerType);
        }
    }));
}

cxx::uniqueptr<CreatureController> CreatureManager::NewControllerInstance(CreatureDefinition* creatureDefinition) const
{
    cxx_assert(creatureDefinition);
    return NewControllerInstance<CreatureController>();
}

void CreatureManager::ProcessRegistrationQueue()
{
    while (!mRegistrationQueue.empty())
    {
        const EntityHandle creatureHandle = mRegistrationQueue.back();

        // get slot
        if (creatureHandle.IsCreature() && (creatureHandle.mIndex < mCreatureSlots.size()))
        {
            CreatureSlot& creatureSlot = mCreatureSlots[creatureHandle.mIndex];
            if (creatureHandle.mGeneration == creatureSlot.mGeneration)
            {
                Creature* creatureInstance = creatureSlot.mCreature.get();
                RegisterCreature(creatureInstance);
            }
            else
            {
                cxx_assert(false);
            }
        }

        mRegistrationQueue.pop_back();
    }
}

void CreatureManager::ProcessRemoveQueue()
{
    while (!mRemoveQueue.empty())
    {
        const EntityHandle creatureHandle = mRemoveQueue.back();

        // get slot
        if (creatureHandle.IsCreature() && (creatureHandle.mIndex < mCreatureSlots.size()))
        {
            CreatureSlot& creatureSlot = mCreatureSlots[creatureHandle.mIndex];
            if (creatureHandle.mGeneration == creatureSlot.mGeneration)
            {
                Creature* creatureInstance = creatureSlot.mCreature.get();
                creatureInstance->DespawnInstance();
                UnregisterCreature(creatureInstance);
                // update slot
                ++creatureSlot.mGeneration;
                cxx_assert(creatureSlot.mGeneration != 0);
                // release creature
                creatureSlot.mCreature.reset();
                creatureSlot.mController.reset();
            }
            else
            {
                cxx_assert(false);
            }
        }
        mRemoveQueue.pop_back();
    }
}

void CreatureManager::DestroyCreatures()
{
    mRegistrationQueue.clear();
    mRemoveQueue.clear();
    mActiveCreatures.clear();
    mActiveCreaturesByType.clear();
    mCreatureSlots.clear();
    mCreatureUidsMap.clear();
}

void CreatureManager::ConfigureNewCreatureInstance(Creature* creature, 
    CreatureController* controller, 
    CreatureDefinition* definition, EntityUid instanceUid, ePlayerID ownerID)
{
    cxx_assert(creature);

    creature->ConfigureInstance(instanceUid, controller, definition, ownerID);
}

void CreatureManager::RegisterCreature(Creature* creatureInstance)
{
    cxx_assert(creatureInstance);

    mActiveCreatures.push_back(creatureInstance);

    const CreatureTypeId creatureTypeId = creatureInstance->GetCreatureTypeId();
    mActiveCreaturesByType[creatureTypeId].push_back(creatureInstance);
}

void CreatureManager::UnregisterCreature(Creature* creatureInstance)
{
    cxx_assert(creatureInstance);

    cxx::erase(mActiveCreatures, creatureInstance);

    mCreatureUidsMap.erase(creatureInstance->GetInstanceUid());

    const CreatureTypeId creatureTypeId = creatureInstance->GetCreatureTypeId();
    cxx::erase(mActiveCreaturesByType[creatureTypeId], creatureInstance);
}
