#pragma once

//////////////////////////////////////////////////////////////////////////

#include "EntityDefs.h"
#include "EntityMsg.h"

//////////////////////////////////////////////////////////////////////////

// base class of all entities in the game world

// defines a minimal set of common data shared by all gameplay objects
// (e.g. transform, id, handle etc.)

//////////////////////////////////////////////////////////////////////////

class Entity: public cxx::noncopyable
{
public:

    // check whether entity marked as pending deletion
    inline bool WasDeleted() const { return mLifecycleFlags.mWasDeleted; }

    // check whether entity was activated in world
    inline bool WasSpawned() const { return mLifecycleFlags.mWasSpawned; }

    // check whether entity was deactivated in world
    inline bool WasDespawned() const { return mLifecycleFlags.mWasDespawned; }

    // accessing internal lifecycle flags
    const EntityLifecycleFlags& GetLifecycleFlags() const 
    { 
        return mLifecycleFlags; 
    }

    // accessing instance handle / unique id
    inline EntityHandle GetOwnHandle() const { return mOwnHandle; }
    inline EntityUid GetInstanceUid() const { return mInstanceUid; }

    // accessing current object transformation
    const EntityTransform& GetTransform() const { return mTransform; }

    inline glm::vec3 GetPosition() const { return mTransform.mPosition; }
    inline glm::vec2 GetPosition2d() const
    {
        return mTransform.GetPosition2d();
    }
    inline cxx::angle_t GetOrientation() const { return mTransform.mOrientation; }

    // handle message
    virtual void ReceiveMsg(EntityMsg& msgData) 
    {
    }

protected:

    Entity() {}
    virtual ~Entity() {}

    // pool
    inline void OnRecycle()
    {
        mLifecycleFlags     = {};
        mOwnHandle          = {};
        mInstanceUid        = {};
        mTransform          = {};
    }

protected:
    EntityLifecycleFlags    mLifecycleFlags {};
    EntityHandle            mOwnHandle;
    EntityUid               mInstanceUid = 0; // unique within game world
    EntityTransform         mTransform; // has no meaning for rooms
};

//////////////////////////////////////////////////////////////////////////

template<typename TComponentsList>
class EnableEntityComponents
{
public:

    // accessing components

    template<typename TComponent>
    inline bool HasComponent() const
    {
        return std::get<std::optional<TComponent>>(mComponents).has_value();
    }

    template<typename TComponent>
    inline const TComponent* GetComponent() const
    {
        const auto& component = std::get<std::optional<TComponent>>(mComponents);
        return component.has_value() ? &(*component) : nullptr;
    }

    template<typename TComponent>
    inline TComponent* GetComponent()
    {
        auto& component = std::get<std::optional<TComponent>>(mComponents);
        return component.has_value() ? &(*component) : nullptr;
    }

    template<typename TComponent>
    inline TComponent* AddComponent()
    {
        auto& component = std::get<std::optional<TComponent>>(mComponents);
        component.emplace();
        return &(*component);
    }

    template<typename TComponent>
    inline void RemoveComponent()
    {
        std::get<std::optional<TComponent>>(mComponents).reset();
    }

protected:
    EnableEntityComponents() {}
    ~EnableEntityComponents() {}

    // pool
    inline void OnRecycle()
    {
        mComponents = {};
    }

protected:
    TComponentsList mComponents;
};

//////////////////////////////////////////////////////////////////////////

template<typename TCapabilitiesList>
class EnableEntityCapabilities
{
public:

    // accessing capabilities

    template<typename TCapability>
    inline bool HasCapability() const 
    { 
        return std::get<TCapability*>(mCapabilities) != nullptr; 
    }

    template<typename TCapability>
    inline TCapability* GetCapability() const
    {
        return std::get<TCapability*>(mCapabilities);
    }

    template<typename TCapability>
    inline void AddCapability(TCapability* capability)
    {
        std::get<TCapability*>(mCapabilities) = capability;
    }

protected:
    EnableEntityCapabilities() {}
    ~EnableEntityCapabilities() {}

    // pool
    inline void OnRecycle()
    {
        mCapabilities = {};
    }

protected:
    TCapabilitiesList mCapabilities;
};

//////////////////////////////////////////////////////////////////////////