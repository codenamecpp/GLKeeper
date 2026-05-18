#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Creature.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class CreatureController: protected GameSessionAware
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

    // find and request next best activity option for creature
    virtual bool SelectNextActivity();

    // process entity notifications
    virtual void HandleNotification(const EntityNotification& notification);

    // creature activity handlers
    virtual bool HandleActivity(CreatureActivity_GoTo& activity);
    virtual bool HandleActivityNotification(CreatureActivity_GoTo& activity, const EntityNotification& notification);

    virtual bool HandleActivity(CreatureActivity_GoToBed& activity);
    virtual bool HandleActivityNotification(CreatureActivity_GoToBed& activity, const EntityNotification& notification);

    virtual bool HandleActivity(CreatureActivity_GoToFood& activity);
    virtual bool HandleActivityNotification(CreatureActivity_GoToFood& activity, const EntityNotification& notification);

    virtual bool HandleActivity(CreatureActivity_Idle& activity);
    virtual bool HandleActivityNotification(CreatureActivity_Idle& activity, const EntityNotification& notification);

    virtual bool HandleActivity(CreatureActivity_Explore& activity);
    virtual bool HandleActivityNotification(CreatureActivity_Explore& activity, const EntityNotification& notification);

    virtual bool HandleActivity(CreatureActivity_Sleep& activity);
    virtual bool HandleActivityNotification(CreatureActivity_Sleep& activity, const EntityNotification& notification);

    virtual bool HandleActivity(CreatureActivity_Eat& activity);
    virtual bool HandleActivityNotification(CreatureActivity_Eat& activity, const EntityNotification& notification);

protected:

    // shortcuts

    inline Creature& GetCreature() const { return *mCreature; }

    // gateway

    // activity
    bool HandleActivity(CreatureActivity_None& activity);
    bool HandleActivityNotification(CreatureActivity_None& activity, const EntityNotification& notification);

    bool HandleActivity(CreatureActivity& activity);
    bool HandleActivityNotification(CreatureActivity& activity, const EntityNotification& notification);

protected:
    Creature* mCreature = nullptr;
};

//////////////////////////////////////////////////////////////////////////