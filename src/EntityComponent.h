#pragma once

// unique identifier of entity component
using EntityComponentID = unsigned int;

// base class of all entity components
class EntityComponent: public cxx::noncopyable
{
    decl_rtti_base(EntityComponent) // enable rtti for components

public:
    const EntityComponentID mComponentID;

    // readonly
    Entity* mParentEntity;

public:
    virtual ~EntityComponent()
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
    EntityComponent(Entity* parentEntity)
        : mParentEntity(parentEntity)
        , mComponentID(NextUniqueID())
    {
        debug_assert(mParentEntity);
    }

private:
    static EntityComponentID NextUniqueID()
    {
        static EntityComponentID IDsCounter = 0;
        return IDsCounter++;
    }
};