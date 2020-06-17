#pragma once

#include "GameDefs.h"
#include "ScenarioDefs.h"
#include "EntityController.h"

// base class of game object
class GameObject: public EntityController
{
public:
    // readonly 
    GameObjectDefinition* mDefinition; // cannot be null
    GameObjectID mID;
    ePlayerID mOwner = ePlayerID_Null;

public:
    GameObject(GameObjectID objectID, GameObjectDefinition* objectDefinition);
    ~GameObject();

    // setup gameobject entity and its components
    void InitGameObjectEntity();
    void FreeGameObjectEntity();

    // process world enter and exit
    void EnterGameWorld();
    void LeaveGameWorld();

protected:
    bool SetAnimationResource(const ArtResource& artResource);

protected:
    Entity* mGameObjectEntity = nullptr;
};