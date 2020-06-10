#include "pch.h"
#include "EntityManager.h"
#include "Entity.h"
#include "TimeManager.h"
#include "TransformComponent.h"

EntityManager gEntityManager;

bool EntityManager::Initialize()
{
    return true;
}

void EntityManager::Deinit()
{
}

void EntityManager::EnterWorld()
{
}

void EntityManager::ClearWorld()
{
    DestroyEntities();
}

void EntityManager::UpdateFrame()
{
    float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();

    bool doCleanup = false;

    // iterating by index here - entity could be added duting update frame
    for (size_t icurrent = 0, Count = mEntitiesList.size(); 
        icurrent < Count; ++icurrent)
    {
        Entity* currEntity = mEntitiesList[icurrent];
        if (currEntity == nullptr)
        {
            doCleanup = true;
            continue;
        }
        currEntity->UpdateFrame(deltaTime);
    }

    if (doCleanup)
    {
        cxx::erase_elements(mEntitiesList, nullptr);
    }
}

void EntityManager::DestroyEntity(Entity* entity)
{
    debug_assert(entity);

    for (size_t icurrent = 0, Count = mEntitiesList.size(); 
        icurrent < Count; ++icurrent)
    {
        if (mEntitiesList[icurrent] == entity)
        {
            mEntitiesList[icurrent] = nullptr;
            break;
        }
    }

    mEntitiesPool.destroy(entity);
}

void EntityManager::DestroyEntities()
{
    for (Entity* currEntity: mEntitiesList)
    {
        if (currEntity)
        {
            mEntitiesPool.destroy(currEntity);
        }
    }

    mEntitiesList.clear();
}

Entity* EntityManager::CreateEntity(const glm::vec3& position, const glm::vec3& direction, float scaling)
{
    Entity* entity = CreateEntity();
    debug_assert(entity);

    TransformComponent* transformComponent = entity->mTransformComponent;
    debug_assert(transformComponent);

    transformComponent->SetPosition(position);
    transformComponent->SetScaling(scaling);

    return entity;
}

Entity* EntityManager::CreateEntity()
{
    Entity* entity = mEntitiesPool.create();
    mEntitiesList.push_back(entity);
    return entity;
}
