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

Entity* EntityRegistry::GetEntityPtr(EntityHandle entityHandle) const
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

bool EntityRegistry::Exists(EntityHandle entityHandle) const
{
    const Entity* entityptr = GetEntityPtr(entityHandle);
    return entityptr && entityptr->Exists();
}

bool EntityRegistry::ExistsOnMap(EntityHandle entityHandle) const
{
    const Entity* entityptr = GetEntityPtr(entityHandle);
    return entityptr && entityptr->ExistsOnMap();
}

bool EntityRegistry::DeleteEntity(EntityHandle entityHandle) const
{   
    bool isSuccess = false;
    switch (entityHandle.mType)
    {
        case eEntityType_None: break;

        case eEntityType_Creature:
            isSuccess = gCreatureManager.DeleteCreature(entityHandle);
        break;

        case eEntityType_GameObject:
            isSuccess = gGameObjectManager.DeleteObject(entityHandle);
        break;

        case eEntityType_Room:
            isSuccess = gRoomManager.DeleteRoom(entityHandle);
        break;
        
        default:
            cxx_assert(false);
        break;
    }

    return isSuccess;
}
