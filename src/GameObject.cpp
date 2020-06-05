#include "pch.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderableComponent.h"

GameObject::GameObject(GameObjectInstanceID instanceID)
    : mDebugColor(Color32_Green)
    , mComponents()
    , mInstanceID(instanceID)
{
    // add automatically transform component
    AddComponent<TransformComponent>();
}

GameObject::~GameObject()
{
    DeleteAllComponents();
}

void GameObject::UpdateFrame(float deltaTime)
{
    for (GameObjectComponent* currComponent: mComponents)
    {
        currComponent->UpdateComponent(deltaTime);
    }
}

void GameObject::UpdateComponentsCache()
{
    // cache transform component
    mTransformComponent = nullptr;
    for (GameObjectComponent* currComponent: mComponents)
    {
        mTransformComponent = cxx::rtti_cast<TransformComponent>(currComponent);
        if (mTransformComponent)
            break;
    }
    
    // cache renderable component
    mRenderableComponent = nullptr;
    for (GameObjectComponent* currComponent: mComponents)
    {
        mRenderableComponent = cxx::rtti_cast<RenderableComponent>(currComponent);
        if (mRenderableComponent)
            break;
    }
}

bool GameObject::HasComponentWithID(GameObjectComponentID componentID) const
{
    if (GameObjectComponent* component = GetComponentByID(componentID))
        return true;

    return false;
}

void GameObject::AttachComponent(GameObjectComponent* component)
{
    debug_assert(component);

    mComponents.push_back(component);
    UpdateComponentsCache();

    component->InitializeComponent();
}

void GameObject::DeleteComponent(GameObjectComponent* component)
{
    if (component)
    {
        cxx::erase_elements(mComponents, component);
        component->DestroyComponent();

        UpdateComponentsCache();
    }
    debug_assert(component);
}

void GameObject::DeleteComponentByID(GameObjectComponentID componentID)
{
    if (GameObjectComponent* component = GetComponentByID(componentID))
    {
        DeleteComponent(component);
    }
}

void GameObject::DeleteAllComponents()
{
    for (GameObjectComponent* currComponent: mComponents)
    {
        currComponent->DestroyComponent();
    }
    mComponents.clear();
    UpdateComponentsCache();
}

GameObjectComponent* GameObject::GetComponentByID(GameObjectComponentID componentID) const
{
    for (GameObjectComponent* currComponent: mComponents)
    {
        if (currComponent->mComponentID == componentID)
            return currComponent;
    }

    return nullptr;
}
