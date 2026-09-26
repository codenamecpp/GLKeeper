#include "stdafx.h"
#include "EntityRegistry.h"
#include "CreatureManager.h"
#include "GameObjectManager.h"
#include "RoomManager.h"

//////////////////////////////////////////////////////////////////////////

EntityRegistry gEntities;

//////////////////////////////////////////////////////////////////////////

EntityRegistry::EntityRegistry()
{
    
}

Entity* EntityRegistry::GetEntityPtr(const EntityHandle& entityHandle) const
{
    Entity* entityptr {};
    switch (entityHandle.mType)
    {
        case eEntityType_None: break;

        case eEntityType_Creature:
            entityptr = gCreatureManager.GetCreaturePtr(entityHandle);
        break;

        case eEntityType_GameObject:
            entityptr = gGameObjectManager.GetObjectPtr(entityHandle);
        break;

        case eEntityType_Room:
            entityptr = gRoomManager.GetRoomPtr(entityHandle);
        break;
        
        default:
            cxx_assert(false);
        break;
    }
    return entityptr;
}

bool EntityRegistry::Exists(const EntityHandle& entityHandle) const
{
    const Entity* entityptr = GetEntityPtr(entityHandle);
    return entityptr && entityptr->Exists();
}

bool EntityRegistry::ExistsOnMap(const EntityHandle& entityHandle) const
{
    const Entity* entityptr = GetEntityPtr(entityHandle);
    return entityptr && entityptr->ExistsOnMap();
}
