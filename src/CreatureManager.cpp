#include "stdafx.h"
#include "CreatureManager.h"
#include "GameWorld.h"
#include "SimplePool.h"
#include "MapUtils.h"
#include "GameSession.h"

#include "CreatureController.h"
#include "WorkerCreatureController.h"

#include "CreatureState_Idle.h"
#include "CreatureState_Working.h"

#include "CreatureAction_IdleStanding.h"
#include "CreatureAction_WalkToPoint.h"
#include "CreatureAction_Wander.h"
#include "CreatureAction_Digging.h"
#include "CreatureAction_FaceTarget.h"
#include "CreatureAction_Mining.h"
#include "CreatureAction_CarryGoldToTreasury.h"
#include "CreatureAction_ReinforceWall.h"
#include "CreatureAction_ClaimFloor.h"
#include "CreatureState_InHand.h"

//////////////////////////////////////////////////////////////////////////

CreatureManager gCreatureManager;

//////////////////////////////////////////////////////////////////////////

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
        if (creature->Exists())
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
        if (creature->Exists())
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
        if (creature->ExistsOnMap())
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

EntityHandle CreatureManager::CreateCreature(CreatureTypeId creatureTypeId, ePlayerID ownerId)
{
    if (CreatureDefinition* definition = gGameSession.GetScenarioDefinition().GetCreatureDefinition(creatureTypeId))
    {
        return CreateCreature(definition, ownerId);
    }
    gConsole.LogMessage(eLogLevel_Warning, "Cannot create creature with type id '%d'", creatureTypeId);
    cxx_assert(false);
    return {};
}

EntityHandle CreatureManager::CreateCreature(CreatureDefinition* definition, ePlayerID ownerId)
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

    const EntityHandle creatureHandle { 
        eEntityType_Creature, definition->mCreatureTypeId, creatureSlot.mGeneration, static_cast<uint32_t>(freeSlotIndex) };
    const EntityUid instanceUid = gGameWorld.GenerateEntityUid();
    mCreatureUidsMap[instanceUid] = creatureHandle;
    ConfigureNewCreatureInstance(creatureSlot.mCreature.get(), creatureSlot.mController.get(), definition, instanceUid, ownerId);
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

bool CreatureManager::ExistsOnMap(const EntityHandle& creatureHandle) const
{
    if (Creature* creature = GetCreaturePtr(creatureHandle))
    {
        return creature->ExistsOnMap();
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
CreatureControllerPtr CreatureManager::NewControllerInstance() const
{
    static SimplePool<TController> controllersPool = {
        [](TController* instance)
        {
            instance->DespawnInstance();
            instance->OnRecycle();
        }};

    CreatureController* controllerInstance = controllersPool.Acquire();
    cxx_assert(controllerInstance);

    return std::move(CreatureControllerPtr (controllerInstance, [](CreatureController* instance)
    {
        if (instance)
        {
            TController* speciticControllerType = static_cast<TController*>(instance);
            controllersPool.Return(speciticControllerType);
        }
    }));
}

CreatureControllerPtr CreatureManager::NewControllerInstance(CreatureDefinition* creatureDefinition) const
{
    cxx_assert(creatureDefinition);

    if (creatureDefinition->mIsWorker)
    {
        return NewControllerInstance<WorkerCreatureController>();
    }

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
    CreatureDefinition* definition, EntityUid instanceUid, ePlayerID ownerId)
{
    cxx_assert(creature);

    creature->ConfigureInstance(instanceUid, controller, definition, ownerId);
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

CreatureStatePtr CreatureManager::CreateState(Creature* creature, eCreatureState stateId) const
{
    cxx_assert(creature);

    CreatureStatePtr stateInstance;
    switch (stateId)
    {
        case eCreatureState_Idle:
            stateInstance = NewStateInstance<CreatureState_Idle>();
        break;
        case eCreatureState_Working:
            stateInstance = NewStateInstance<CreatureState_Working>();
        break;
        case eCreatureState_Stunned:
        break;
        case eCreatureState_Frozen:
        break;
        case eCreatureState_Unconscious:
        break;
        case eCreatureState_Tortured:
        break;
        case eCreatureState_Dead:
        break;
        case eCreatureState_InHand:
            stateInstance = NewStateInstance<CreatureState_InHand>();
        break;
        case eCreatureState_InPrison:
        break;
        case eCreatureState_Dropped:
        break;
        case eCreatureState_Slapped:
        break;
        case eCreatureState_GetUp:
        break;
        case eCreatureState_EnteringDungeon:
        break;
    }
    cxx_assert(stateInstance);
    if (stateInstance)
    {
        stateInstance->Configure(creature);
    }
    return stateInstance;
}


template<typename TState>
CreatureStatePtr CreatureManager::NewStateInstance() const
{
    static SimplePool<TState> instancesPool = {
        [](TState* instance)
        {
            instance->OnRecycle();
        }};
    CreatureState* stateInstance = instancesPool.Acquire();
    cxx_assert(stateInstance);
    return std::move(CreatureStatePtr (stateInstance, 
        [](CreatureState* instance)
        {
            if (instance)
            {
                TState* speciticStateType = static_cast<TState*>(instance);
                instancesPool.Return(speciticStateType);
            }
        }));
}


template<typename TAction, typename... TArgs>
CreatureActionPtr CreatureManager::NewActionInstance(TArgs && ... args) const
{
    static SimplePool<TAction> instancesPool = {
        [](TAction* instance)
        {
            instance->OnRecycle();
        }};
    TAction* newInstance = instancesPool.Acquire();
    newInstance->Configure(std::forward<TArgs>(args)...);
    cxx_assert(newInstance);
    return std::move(CreatureActionPtr (newInstance, 
        [](CreatureAction* instance)
        {
            if (instance)
            {
                TAction* speciticAction = static_cast<TAction*>(instance);
                instancesPool.Return(speciticAction);
            }
        }));
}

CreatureActionPtr CreatureManager::CreateIdleStandingAction(Creature* creature)
{
    CreatureActionPtr instance = NewActionInstance<CreatureAction_IdleStanding>(creature);
    cxx_assert(instance);
    return instance;
}

CreatureActionPtr CreatureManager::CreateWanderAction(Creature* creature, const glm::vec2& destination)
{
    CreatureActionPtr instance = NewActionInstance<CreatureAction_Wander>(creature, destination);
    cxx_assert(instance);
    return instance;
}

CreatureActionPtr CreatureManager::CreateWalkToPointAction(Creature* creature, const glm::vec2& destination)
{
    CreatureActionPtr instance = NewActionInstance<CreatureAction_WalkToPoint>(creature, destination);
    cxx_assert(instance);
    return instance;
}

CreatureActionPtr CreatureManager::CreateDiggingAction(Creature* creature, const glm::vec2& workPoint, const Point2D& targetTile)
{
    CreatureActionPtr instance = NewActionInstance<CreatureAction_Digging>(creature, workPoint, targetTile);
    cxx_assert(instance);
    return instance;
}

CreatureActionPtr CreatureManager::CreateFaceTileAction(Creature* creature, const Point2D& targetTile)
{
    CreatureActionPtr instance = NewActionInstance<CreatureAction_FaceTarget>(creature, targetTile);
    cxx_assert(instance);
    return instance;
}

CreatureActionPtr CreatureManager::CreateMiningAction(Creature* creature, const glm::vec2& workPoint, const Point2D& targetTile)
{
    CreatureActionPtr instance = NewActionInstance<CreatureAction_Mining>(creature, workPoint, targetTile);
    cxx_assert(instance);
    return instance;
}

CreatureActionPtr CreatureManager::CreateCarryGoldToTreasuryAction(Creature* creature, const Point2D& targetTile)
{
    CreatureActionPtr instance = NewActionInstance<CreatureAction_CarryGoldToTreasury>(creature, targetTile);
    cxx_assert(instance);
    return instance;
}

CreatureActionPtr CreatureManager::CreateReinforceWallAction(Creature* creature, const glm::vec2& workPoint, const Point2D& targetTile)
{
    CreatureActionPtr instance = NewActionInstance<CreatureAction_ReinforceWall>(creature, workPoint, targetTile);
    cxx_assert(instance);
    return instance;
}

CreatureActionPtr CreatureManager::CreateClaimFloorAction(Creature* creature, const Point2D& targetTile)
{
    CreatureActionPtr instance = NewActionInstance<CreatureAction_ClaimFloor>(creature, targetTile);
    cxx_assert(instance);
    return instance;
}
