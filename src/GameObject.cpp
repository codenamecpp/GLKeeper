#include "pch.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderableComponent.h"

GameObject::GameObject(GameObjectInstanceID instanceID)
    : mDebugColor(Color32_Green)
    , mIsAttachedToScene()
    , mComponents()
    , mInstanceID(instanceID)
{
    // add automatically transform component
    TransformComponent* transformComponent = new TransformComponent(this);
    AddComponent(transformComponent);
}

GameObject::~GameObject()
{
    // entity must be detached from scene before destroy
    debug_assert(IsAttachedToScene() == false);

    DeleteAllComponents();
}

void GameObject::UpdateFrame(float deltaTime)
{
    for (GameObjectComponent* currComponent: mComponents)
    {
        currComponent->UpdateComponent(deltaTime);
    }
}

void GameObject::SetAttachedToScene(bool isAttached)
{
    mIsAttachedToScene = isAttached;
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

bool GameObject::IsAttachedToScene() const
{
    return mIsAttachedToScene;
}

bool GameObject::AddComponent(GameObjectComponent* component)
{
    if (component == nullptr)
    {
        debug_assert(false);
        return false;
    }

    // check if component already added to gameobject
    if (HasComponentWithID(component->mComponentID))
    {
        debug_assert(false);
        return true;
    }

    mComponents.push_back(component);
    UpdateComponentsCache();
    return true;
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
