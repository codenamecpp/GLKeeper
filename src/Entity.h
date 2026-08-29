#pragma once

//////////////////////////////////////////////////////////////////////////

#include "EntityDefs.h"
#include "EntityMsg.h"
#include "EntityComponents.h"
#include "EntityCapabilities.h"

//////////////////////////////////////////////////////////////////////////

// base class of all entities in the game world

// defines a minimal set of common data shared by all gameplay objects
// (e.g. transform, id, handle etc.)

//////////////////////////////////////////////////////////////////////////

class Entity: public cxx::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////

    // check whether entity marked as pending deletion
    inline bool WasDeleted() const { return mEntityFlags.mWasDeleted; }

    // check whether entity was activated in world
    inline bool WasSpawned() const { return mEntityFlags.mWasSpawned; }

    // check whether entity was deactivated in world
    inline bool WasDespawned() const { return mEntityFlags.mWasDespawned; }

    // check whether entity was spawned and has not been despawned or deleted
    inline bool Exists() const
    {
        return mEntityFlags.mWasSpawned && 
            !mEntityFlags.mWasDespawned &&
            !mEntityFlags.mWasDeleted;
    }
    // same as Exists and placed on game map
    inline bool ExistsOnMap() const
    {
        return mEntityFlags.mWasSpawned &&  
            !mEntityFlags.mWasDespawned && 
            !mEntityFlags.mWasDeleted &&
            !mEntityFlags.mIsUnplaced;
    }

    //////////////////////////////////////////////////////////////////////////

    // accessing instance handle / unique id
    inline EntityHandle GetOwnHandle() const { return mOwnHandle; }
    inline EntityUid GetInstanceUid() const { return mInstanceUid; }

    // accessing owner player
    // the owner may change as a result of conversion
    inline ePlayerID GetOwnerId() const { return mOwnerId; }

    inline bool HasOwner(ePlayerID playerId) const { return mOwnerId == playerId; }

    //////////////////////////////////////////////////////////////////////////

    // accessing current object transformation
    const EntityTransform& GetTransform() const { return mTransform; }
    inline glm::vec3 GetPosition() const { return mTransform.mPosition; }
    inline glm::vec2 GetPosition2d() const
    {
        return mTransform.GetPosition2d();
    }
    inline cxx::angle_t GetOrientation() const { return mTransform.mOrientation; }

    //////////////////////////////////////////////////////////////////////////
    // components
    //////////////////////////////////////////////////////////////////////////

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
        cxx_assert(!HasComponent<TComponent>());
        auto& component = std::get<std::optional<TComponent>>(mComponents);
        component.emplace();
        return &(*component);
    }

    template<typename TComponent>
    inline void RemoveComponent()
    {
        std::get<std::optional<TComponent>>(mComponents).reset();
    }

    //////////////////////////////////////////////////////////////////////////
    // capabilities
    //////////////////////////////////////////////////////////////////////////

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
    inline void SetCapability(TCapability* capability)
    {
        cxx_assert(!HasCapability<TCapability>());
        std::get<TCapability*>(mCapabilities) = capability;
    }

    //////////////////////////////////////////////////////////////////////////

    // handle message
    virtual void ReceiveMsg(EntityMsg& msgData) 
    {
    }

protected:
    Entity() = default;
    virtual ~Entity() {}

    // pool
    inline void OnRecycle()
    {
        mEntityFlags = {};
        mOwnHandle      = {};
        mInstanceUid    = {};
        mTransform      = {};
        mOwnerId        = ePlayerID_Neutral;
        mCapabilities   = {};
        // reset components
        std::apply([](auto&... roller) {((roller.reset()), ...);}, mComponents);
    }

    // helpers

    inline void SetEntityUnplaced(bool isUnplaced)
    {
        mEntityFlags.mIsUnplaced = isUnplaced;
    }

protected:
    EntityHandle    mOwnHandle;
    EntityUid       mInstanceUid = 0; // unique within game world
    EntityFlags     mEntityFlags {};
    ePlayerID       mOwnerId = ePlayerID_Neutral;
    EntityTransform mTransform; // has no meaning for rooms

    //////////////////////////////////////////////////////////////////////////

    using TComponentsList = std::tuple<
            std::optional<MoneyComponent>
        >;
    TComponentsList mComponents;

    //////////////////////////////////////////////////////////////////////////

    using TCapabilitiesList = std::tuple<
            MoneyStorageRoomCapability*,
            ObjectStorageRoomCapability*,
            GoldContainerCapability*
        >;
    TCapabilitiesList mCapabilities;
};

//////////////////////////////////////////////////////////////////////////