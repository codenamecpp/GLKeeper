#pragma once

//////////////////////////////////////////////////////////////////////////

#include "EntityDefs.h"

//////////////////////////////////////////////////////////////////////////

class EntityRegistry final: public cxx::noncopyable
{
public:
    EntityRegistry();

    // common shortcuts
    bool Exists(EntityHandle entityHandle) const;
    bool ExistsOnMap(EntityHandle entityHandle) const;
    bool DeleteEntity(EntityHandle entityHandle) const;

    // accessing entity capabilities shortcuts
    template<typename TCapability>
    inline TCapability* GetEntityCapability(EntityHandle entityHandle) const
    {
        if (Entity* entity = GetEntityPtr(entityHandle))
            return entity->GetCapability<TCapability>();

        return nullptr;
    }

    // accessing entity components shortcuts
    template<typename TComponent>
    inline TComponent* GetEntityComponent(EntityHandle entityHandle) const
    {
        if (Entity* entity = GetEntityPtr(entityHandle))
            return entity->GetComponent<TComponent>();

        return nullptr;
    }

    Entity* GetEntityPtr(EntityHandle entityHandle) const;

private:
    
};

//////////////////////////////////////////////////////////////////////////

extern EntityRegistry gEntities;

//////////////////////////////////////////////////////////////////////////