#pragma once

//////////////////////////////////////////////////////////////////////////

#include "EntityDefs.h"

//////////////////////////////////////////////////////////////////////////

struct EntityMsg
{
public:

    //////////////////////////////////////////////////////////////////////////

    enum eID
    {
        eID_None,

        // physics
        eID_SyncWithPhysicsTransform,

        // locomotion
        eID_LocoApplyVelocities,
        eID_LocoClearVelocities
    };

    //////////////////////////////////////////////////////////////////////////

public:
    EntityMsg(eID notificationID)
        : mID(notificationID)
    {
    }

    inline bool Is(eID messageID) const { return mID == messageID; }

    inline bool WasConsumed() const { return mIsConsumed; }
    inline void SetConsumed()
    {
        mIsConsumed = true;
    }
public:
    const eID mID;

    // valid for eNotificationID_LocoApplyVelocities
    struct 
    {
        glm::vec2 mLinear {};
        float mAngular {};
    } 
    mLocoVelocities;

    bool mIsConsumed {};
};

//////////////////////////////////////////////////////////////////////////

struct EntityMsg_SyncWithPhysics: public EntityMsg
{
public:
    EntityMsg_SyncWithPhysics()
        : EntityMsg(eID_SyncWithPhysicsTransform)
    {
    }
};

//////////////////////////////////////////////////////////////////////////

struct EntityMsg_LocoApplyVelocities: public EntityMsg
{
public:
    EntityMsg_LocoApplyVelocities(const glm::vec2 linear, float angular)
        : EntityMsg(eID_LocoApplyVelocities)
    {
        mLocoVelocities.mLinear = linear;
        mLocoVelocities.mAngular = angular;
    }
};

struct EntityMsg_LocoClearVelocities: public EntityMsg
{
public:
    EntityMsg_LocoClearVelocities()
        : EntityMsg(eID_LocoClearVelocities)
    {
    }
};

//////////////////////////////////////////////////////////////////////////