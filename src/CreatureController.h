#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Creature.h"

//////////////////////////////////////////////////////////////////////////

class CreatureController: public cxx::noncopyable
{
public:
    CreatureController() = default;
    virtual ~CreatureController();

    // performs early initialization of the instance, before it is spawned
    virtual void ConfigureInstance(Creature* creatureInstance);

    // preloads resources, adds the instance to the game world and activates it
    virtual void SpawnInstance();

    // deactivates the instance and releases its internal resources
    virtual void DespawnInstance();

    // pool
    virtual void OnRecycle();

    // variable fps update
    virtual void UpdateFrame(float deltaTime);

    // fixed logic tick update
    virtual void UpdateLogic(float stepDeltaTime);

    // animation states should be initialized once when creature is spawned
    virtual void ConfigureCreatureAnimationStates();

    // process entity messages
    virtual void HandleMessage(const EntityMsg& msgData);

protected:

    // shortcuts

    inline Creature& GetCreature() const { return *mCreature; }

protected:
    Creature* mCreature = nullptr;
};

//////////////////////////////////////////////////////////////////////////