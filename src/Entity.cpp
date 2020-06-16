#include "pch.h"
#include "Entity.h"
#include "TransformComponent.h"
#include "RenderableComponent.h"

Entity::Entity()
    : mDebugColor(Color32_Green)
    , mComponents()
{
    // automatically add transform component
    AddComponent<TransformComponent>();
}

Entity::~Entity()
{
    RemoveAllComponents();
}

void Entity::UpdateComponents(float deltaTime)
{
    bool pendingDeletion = false;
    for (EntityComponent* currComponent: mComponents)
    {
        if (currComponent->IsComponentDeleted())
        {
            pendingDeletion = true;
            continue;
        }

        if (currComponent->IsComponentActive())
        {
            currComponent->UpdateComponent(deltaTime);
        }
    }

    if (pendingDeletion)
    {
        for (auto curr_iterator = mComponents.begin(); curr_iterator != mComponents.end(); )
        {
            EntityComponent* currComponent = *curr_iterator;
            if (currComponent->IsComponentDeleted())
            {
                curr_iterator = mComponents.erase(curr_iterator);
                currComponent->DestroyComponent();
                continue;
            }
            ++curr_iterator;
        }
    }
}

void Entity::SetActive(bool isActive)
{
    for (EntityComponent* currComponent: mComponents)
    {
        if (currComponent->IsComponentDeleted())
            continue;

        currComponent->EnableComponent(isActive);
    }
}

void Entity::UpdateComponentsCache()
{
    // cache transform component
    mTransform = GetComponent<TransformComponent>();

    // cache renderable component
    mRenderable = GetComponent<RenderableComponent>();
}

void Entity::AttachComponent(EntityComponent* component)
{
    debug_assert(component);

    mComponents.push_back(component);
    UpdateComponentsCache();

    component->AwakeComponent();
}

void Entity::RemoveComponent(EntityComponent* component)
{
    if (component)
    {
        cxx::erase_elements(mComponents, component);
        component->DestroyComponent();

        UpdateComponentsCache();
    }
    debug_assert(component);
}

void Entity::RemoveAllComponents()
{
    for (EntityComponent* currComponent: mComponents)
    {
        currComponent->DestroyComponent();
    }
    mComponents.clear();
    UpdateComponentsCache();
}

bool Entity::IsActive() const
{
    for (EntityComponent* currComponent: mComponents)
    {
        if (currComponent->IsComponentDeleted())
            continue;

        if (currComponent->IsComponentActive())
            return true;
    }

    return false;
}

void Entity::SetController(EntityController* controller)
{
    mController = controller;
}
