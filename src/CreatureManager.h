#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Creature.h"

//////////////////////////////////////////////////////////////////////////

class CreatureManager final: public cxx::noncopyable
{
private:

    //////////////////////////////////////////////////////////////////////////
    struct CreatureSlot { uint32_t mGeneration = 1; 
        cxx::uniqueptr<Creature> mCreature; 
        CreatureControllerPtr mController;
    };
    //////////////////////////////////////////////////////////////////////////

public:
    bool LoadScenario(const ScenarioDefinition& scenarioDef);
    void EnterWorld();
    void ClearWorld();

    void UpdateFrame(float deltaTime);
    void UpdateLogic(float stepDeltaTime);
    void UpdatePhysics(float stepDeltaTime);

    // forcibly refresh active creature lists, destroy creatures pending destruction
    // note: this will invalidate list iterators
    void ProcessCreatureChanges();

    EntityHandle CreateScenarioCreature(const ScenarioCreatureThing& creatureThing);
    EntityHandle CreateCreature(CreatureTypeId creatureTypeId, ePlayerID ownerId);
    EntityHandle CreateCreature(CreatureDefinition* definition, ePlayerID ownerId);

    EntityHandle FindCreature(EntityUid creatureUid) const;

    Creature* GetCreaturePtr(EntityUid creatureUid) const;
    Creature* GetCreaturePtr(const EntityHandle& creatureHandle) const;

    bool ActivateCreature(const EntityHandle& creatureHandle);
    bool ActivateCreature(EntityUid creatureUid);

    bool DeleteCreature(const EntityHandle& creatureHandle);
    bool DeleteCreature(EntityUid creatureUid);

    // check whether the creature was spawned and remains active on map
    bool ExistsOnMap(const EntityHandle& creatureHandle) const;

    // get all currently active creatures in game world
    // warning: do not store result
    inline cxx::span<Creature*> GetCreatures() const { return mActiveCreatures; }
    inline cxx::span<Creature*> GetCreaturesByType(CreatureTypeId creatureTypeId) const
    {
        auto map_it = mActiveCreaturesByType.find(creatureTypeId);
        if (map_it != mActiveCreaturesByType.end())
        {
            return map_it->second;
        }
        return {};
    }

public:

    //////////////////////////////////////////////////////////////////////////

    // states factory

    CreatureStatePtr CreateState(Creature* creature, eCreatureState stateId) const;

    //////////////////////////////////////////////////////////////////////////

    // actions factory

    CreatureActionPtr CreateIdleStandingAction(Creature* creature);
    CreatureActionPtr CreateWanderAction(Creature* creature, const glm::vec2& destination);
    CreatureActionPtr CreateWalkToPointAction(Creature* creature, const glm::vec2& destination);
    CreatureActionPtr CreateDiggingAction(Creature* creature, const glm::vec2& workPoint, const Point2D& targetTile);
    CreatureActionPtr CreateMiningAction(Creature* creature, const glm::vec2& workPoint, const Point2D& targetTile);
    CreatureActionPtr CreateFaceTileAction(Creature* creature, const Point2D& targetTile);
    CreatureActionPtr CreateCarryGoldToTreasuryAction(Creature* creature, const Point2D& targetTile);
    CreatureActionPtr CreateReinforceWallAction(Creature* creature, const glm::vec2& workPoint, const Point2D& targetTile);
    CreatureActionPtr CreateClaimFloorAction(Creature* creature, const Point2D& targetTile);

    //////////////////////////////////////////////////////////////////////////


private:
    // factory
    cxx::uniqueptr<Creature> NewCreatureInstance() const;

    template<typename TState>
    CreatureStatePtr NewStateInstance() const;

    template<typename TAction, typename... TArgs>
    CreatureActionPtr NewActionInstance(TArgs && ... args) const;

    template<typename TController>
    CreatureControllerPtr NewControllerInstance() const;
    CreatureControllerPtr NewControllerInstance(CreatureDefinition* creatureDefinition) const;

    void ProcessRegistrationQueue();
    void ProcessRemoveQueue();

    void DestroyCreatures();

    void ConfigureNewCreatureInstance(Creature* creature, 
        CreatureController* controller, 
        CreatureDefinition* definition, EntityUid instanceUid, ePlayerID ownerId);

    void RegisterCreature(Creature* creatureInstance);
    void UnregisterCreature(Creature* creatureInstance);

private:
    using CreaturesList = std::vector<Creature*>;
    std::vector<CreatureSlot> mCreatureSlots;
    std::unordered_map<EntityUid, EntityHandle> mCreatureUidsMap;
    std::unordered_map<CreatureTypeId, CreaturesList> mActiveCreaturesByType;
    CreaturesList mActiveCreatures;
    std::vector<EntityHandle> mRegistrationQueue; // pending registration in lists
    std::vector<EntityHandle> mRemoveQueue; // pending destroy
};

//////////////////////////////////////////////////////////////////////////

extern CreatureManager gCreatureManager;

//////////////////////////////////////////////////////////////////////////