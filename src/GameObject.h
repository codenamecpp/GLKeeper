#pragma once

#include "GameDefs.h"
#include "ScenarioDefs.h"
#include "Entity.h"

// base class of game object
class GameObject: public Entity
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

    // override Entity methods
    void EnterGameWorld() override;
    void LeaveGameWorld() override;
    void UpdateFrame() override;

protected:
    bool SetAnimationResource(const ArtResource& artResource);

protected:
    RenderableModel* mSceneObject = nullptr;
};