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
        currComponent->UpdateComponent(deltaTime);
    }
}

void Entity::UpdateComponentsCache()
{
    // cache transform component
    mTransformComponent = nullptr;
    for (EntityComponent* currComponent: mComponents)
    {
        mTransformComponent = cxx::rtti_cast<TransformComponent>(currComponent);
        if (mTransformComponent)
            break;
    }
    
    // cache renderable component
    mRenderableComponent = nullptr;
    for (EntityComponent* currComponent: mComponents)
    {
        mRenderableComponent = cxx::rtti_cast<RenderableComponent>(currComponent);
        if (mRenderableComponent)
            break;
    }
}

bool Entity::HasComponentWithID(EntityComponentID componentID) const
{
    if (EntityComponent* component = GetComponentByID(componentID))
        return true;

    return false;
}

void Entity::AttachComponent(EntityComponent* component)
{
    debug_assert(component);

    mComponents.push_back(component);
    UpdateComponentsCache();

    component->InitializeComponent();
}

void Entity::DeleteComponent(EntityComponent* component)
{
    if (component)
    {
        cxx::erase_elements(mComponents, component);
        component->DestroyComponent();

        UpdateComponentsCache();
    }
    debug_assert(component);
}

void Entity::DeleteComponentByID(EntityComponentID componentID)
{
    if (EntityComponent* component = GetComponentByID(componentID))
    {
        DeleteComponent(component);
    }
}

void Entity::DeleteAllComponents()
{
    for (EntityComponent* currComponent: mComponents)
    {
        currComponent->DestroyComponent();
    }
    mComponents.clear();
    UpdateComponentsCache();
}

EntityComponent* Entity::GetComponentByID(EntityComponentID componentID) const
{
    for (EntityComponent* currComponent: mComponents)
    {
        if (currComponent->mComponentID == componentID)
            return currComponent;
    }

    return nullptr;
}
