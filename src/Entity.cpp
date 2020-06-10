#include "pch.h"
#include "Entity.h"
#include "TransformComponent.h"
#include "RenderableComponent.h"

Entity::Entity()
    : mDebugColor(Color32_Green)
    , mComponents()
{
    // add automatically transform component
    AddComponent<TransformComponent>();
}

Entity::~Entity()
{
    DeleteAllComponents();
}

void Entity::UpdateFrame(float deltaTime)
{
    for (EntityComponent* currComponent: mComponents)
    {
        if (currComponent->IsComponentActive())
        {
            currComponent->UpdateComponent(deltaTime);
        }
    }
}

void Entity::SetActive(bool isActive)
{
    for (EntityComponent* currComponent: mComponents)
    {
        currComponent->EnableComponent(isActive);
    }
}

void Entity::UpdateComponentsCache()
{
    // cache transform component
    mTransform = nullptr;
    for (EntityComponent* currComponent: mComponents)
    {
        mTransform = cxx::rtti_cast<TransformComponent>(currComponent);
        if (mTransform)
            break;
    }
    
    // cache renderable component
    mRenderable = nullptr;
    for (EntityComponent* currComponent: mComponents)
    {
        mRenderable = cxx::rtti_cast<RenderableComponent>(currComponent);
        if (mRenderable)
            break;
    }
}

void Entity::AttachComponent(EntityComponent* component)
{
    debug_assert(component);

    mComponents.push_back(component);
    UpdateComponentsCache();

    component->AwakeComponent();
}

void Entity::DeleteComponent(EntityComponent* component)
{
    if (component)
    {
        cxx::erase_elements(mComponents, component);
        component->DeleteComponent();

        UpdateComponentsCache();
    }
    debug_assert(component);
}

void Entity::DeleteAllComponents()
{
    for (EntityComponent* currComponent: mComponents)
    {
        currComponent->DeleteComponent();
    }
    mComponents.clear();
    UpdateComponentsCache();
}

bool Entity::IsActive() const
{
    for (EntityComponent* currComponent: mComponents)
    {
        if (currComponent->IsComponentActive())
            return true;
    }

    return false;
}