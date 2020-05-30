#pragma once

// base class of all gameobject components
class GameObjectComponent: public cxx::noncopyable
{
    decl_rtti_base(GameObjectComponent) // enable rtti for components

public:
    // readonly
    GameObject* mGameObject;

public:
    virtual ~GameObjectComponent()
    {
    }
    // process update
    // @param deltaTime: Time since last update
    virtual void UpdateComponent(float deltaTime)
    {
        // do nothing
    }
    // destroy gameobject component instance
    virtual void DestroyComponent()
    {
        delete this;
    }
protected:
    // base component does not meant to be instantiated directly
    GameObjectComponent(GameObject* sceneObject)
        : mGameObject(sceneObject)
    {
        debug_assert(mGameObject);
    }
};