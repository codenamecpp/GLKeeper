#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObject.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class GameObjectController: protected GameSessionAware
{
public:
    GameObjectController() = default;
    virtual ~GameObjectController();

    // performs early initialization of the instance, before it is spawned
    virtual void ConfigureInstance(GameObject* objectInstance);

    // preloads resources, adds the instance to the game world and activates it
    virtual void SpawnInstance();

    // deactivates the instance and releases its internal resources
    virtual void DespawnInstance();

    // pool
    virtual void OnRecycle();

    // fixed logic tick update
    virtual void UpdateLogic(float stepDeltaTime);

    virtual void ParentRoomChanged();

protected:
    inline GameObject& GetGameObject() const { return *mGameObject; }

private:
    GameObject* mGameObject = nullptr;
};