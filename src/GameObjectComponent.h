#pragma once

// unique gameobject component identifier
using GameObjectComponentID = unsigned int;

// base class of all gameobject components
class GameObjectComponent: public cxx::noncopyable
{
    decl_rtti_base(GameObjectComponent) // enable rtti for components

public:
    const GameObjectComponentID mComponentID;

    // readonly
    GameObject* mGameObject;

public:
    virtual ~GameObjectComponent()
    {
    }

    // try to cast base component type to derived
    template<typename TComponent>
    inline TComponent* CastComponent()
    {
        return cxx::rtti_cast<TComponent>(this);
    }

    // process update
    // @param deltaTime: Time since last update
    virtual void UpdateComponent(float deltaTime)
    {
        // do nothing
    }
    // component post construction
    virtual void InitializeComponent()
    {
    }
    // destroy component instance
    virtual void DestroyComponent()
    {
        delete this;
    }
protected:
    // base component does not meant to be instantiated directly
    GameObjectComponent(GameObject* sceneObject)
        : mGameObject(sceneObject)
        , mComponentID(NextUniqueID())
    {
        debug_assert(mGameObject);
    }

private:
    static GameObjectComponentID NextUniqueID()
    {
        static GameObjectComponentID IDsCounter = 0;
        return IDsCounter++;
    }
};