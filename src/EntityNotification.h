#pragma once

//////////////////////////////////////////////////////////////////////////

#include "EntityDefs.h"

//////////////////////////////////////////////////////////////////////////

struct EntityNotification
{
public:
    enum eID
    {
        eID_None,
        // physics
        eID_SyncWithPhysicsTransform,
        // locomotion
        eID_LocoApplyVelocities,
        eID_LocoClearVelocities
    };
public:
    EntityNotification(eID notificationID)
        : mID(notificationID)
    {
    }

    inline bool Is(eID notificationID) const { return mID == notificationID; }

    //////////////////////////////////////////////////////////////////////////
    // helpers
    //////////////////////////////////////////////////////////////////////////

    static EntityNotification ForSyncWithPhysics()
    {
        return EntityNotification{eID_SyncWithPhysicsTransform};
    }
    static EntityNotification ForLocoApplyVelocities(const glm::vec2 linear, float angular)
    {
        EntityNotification notification {eID_LocoApplyVelocities};
        notification.mLocoVelocities.mLinear = linear;
        notification.mLocoVelocities.mAngular = angular;
        return notification;
    }
    static EntityNotification ForLocoClearVelocities()
    {
        return EntityNotification{eID_LocoClearVelocities};
    }

public:
    eID mID;

    // valid for eNotificationID_LocoApplyVelocities
    struct 
    {
        glm::vec2 mLinear {};
        float mAngular {};
    } 
    mLocoVelocities;
};

//////////////////////////////////////////////////////////////////////////