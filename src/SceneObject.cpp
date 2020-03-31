#include "pch.h"
#include "SceneObject.h"
#include "RenderScene.h"

SceneObject::SceneObject()
    : mDebugColor(Color32_Green)
    , mIsAttachedToScene()
    , mComponents()
{
}

SceneObject::~SceneObject()
{
    // entity must be detached from scene before destroy
    debug_assert(IsAttachedToScene() == false);

    DeleteAllComponents();
}

void SceneObject::UpdateFrame(float deltaTime)
{
    for (SceneObjectComponent* currComponent: mComponents)
    {
        if (currComponent && currComponent->IsUpdatableComponent())
        {
            currComponent->UpdateFrame(deltaTime);
        }
    }
}

void SceneObject::SetAttachedToScene(bool isAttached)
{
    mIsAttachedToScene = isAttached;
}

bool SceneObject::IsAttachedToScene() const
{
    return mIsAttachedToScene;
}

void SceneObject::AddComponent(SceneObjectComponent* component)
{
    if (component == nullptr)
    {
        debug_assert(false);
        return;
    }

    eSceneObjectComponent componentid = component->mComponentType;
    if (HasComponent(componentid))
    {
        // destroy previous component of this type
        DeleteComponent(componentid);
    }

    mComponents[componentid] = component;
}

void SceneObject::DeleteComponent(SceneObjectComponent* component)
{
    if (component == nullptr)
    {
        debug_assert(false);
        return;
    }
    eSceneObjectComponent componentid = component->mComponentType;
    if (mComponents[componentid])
    {
        DeleteComponent(componentid);
    }
}

void SceneObject::DeleteComponent(eSceneObjectComponent componentId)
{
    debug_assert(componentId < eSceneObjectComponent_Count);
    
    if (mComponents[componentId])
    {
        SafeDelete(mComponents[componentId]);
    }
}

void SceneObject::DeleteAllComponents()
{
    for (int icurr = 0; icurr < eSceneObjectComponent_Count; ++icurr)
    {
        SafeDelete(mComponents[icurr]);
    }
}

void SceneObject::DeleteRenderableComponents()
{
    for (int icurr = 0; icurr < eSceneObjectComponent_Count; ++icurr)
    {
        if (icurr == eSceneObjectComponent_Transform)
            continue;

        SafeDelete(mComponents[icurr]);
    }
}

bool SceneObject::HasComponent(eSceneObjectComponent componentId) const
{
    debug_assert(componentId < eSceneObjectComponent_Count);

    if (mComponents[componentId])
    {
        return true;
    }

    return false;
}

