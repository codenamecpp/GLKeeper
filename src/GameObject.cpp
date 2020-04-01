#include "pch.h"
#include "GameObject.h"

GameObject::GameObject(GameObjectInstanceID instanceID)
    : mDebugColor(Color32_Green)
    , mIsAttachedToScene()
    , mComponents()
    , mInstanceID(instanceID)
{
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
        if (currComponent && currComponent->IsUpdatableComponent())
        {
            currComponent->UpdateFrame(deltaTime);
        }
    }
}

void GameObject::SetAttachedToScene(bool isAttached)
{
    mIsAttachedToScene = isAttached;
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

    eGameObjectComponent componentid = component->mComponentType;
    if (HasComponent(componentid))
    {
        debug_assert(false);
        return false;
    }

    mComponents[componentid] = component;
    return true;
}

void GameObject::DeleteComponent(GameObjectComponent* component)
{
    if (component == nullptr)
    {
        debug_assert(false);
        return;
    }
    eGameObjectComponent componentid = component->mComponentType;
    if (mComponents[componentid])
    {
        DeleteComponent(componentid);
    }
}

void GameObject::DeleteComponent(eGameObjectComponent componentId)
{
    debug_assert(componentId < eGameObjectComponent_Count);

    if (mComponents[componentId])
    {
        SafeDelete(mComponents[componentId]);
    }
}

void GameObject::DeleteAllComponents()
{
    for (int icurr = 0; icurr < eGameObjectComponent_Count; ++icurr)
    {
        SafeDelete(mComponents[icurr]);
    }
}

void GameObject::DeleteRenderableComponents()
{
    for (int icurr = 0; icurr < eGameObjectComponent_Count; ++icurr)
    {
        if (mComponents[icurr] && mComponents[icurr]->IsRenderableComponent())
        {
            SafeDelete(mComponents[icurr]);
        }
    }
}

bool GameObject::HasComponent(eGameObjectComponent componentId) const
{
    debug_assert(componentId < eGameObjectComponent_Count);

    if (mComponents[componentId])
    {
        return true;
    }

    return false;
}

bool GameObject::HasUpdatableComponents() const
{
    for (GameObjectComponent* currComponent: mComponents)
    {
        if (currComponent && currComponent->IsUpdatableComponent())
            return true;
    }
    return false;
}

bool GameObject::HasRenderableComponents() const
{
    for (GameObjectComponent* currComponent: mComponents)
    {
        if (currComponent && currComponent->IsRenderableComponent())
            return true;
    }
    return false;
}
