#pragma once

#include "GameObjectComponent.h"
#include "GameDefs.h"

class GameObject: public cxx::noncopyable
{
public:
    Color32 mDebugColor; // color used for debug draw

    float mDistanceToCameraSquared; // this value gets updated during scene rendition

    // readonly
    const GameObjectInstanceID mInstanceID;

    // components cache
    TransformComponent* mTransformComponent = nullptr;
    RenderableComponent* mRenderableComponent = nullptr;

public:
    GameObject(GameObjectInstanceID instanceID);
    ~GameObject();

    // process game object update frame
    // @param deltaTime: Time since last update
    void UpdateFrame(float deltaTime);

    // add or remove object component, object takes ownership on pointer
    // @param component: Component instance
    template<typename TComponent, typename... TArgs>
    inline TComponent* AddComponent(const TArgs &&...  constructionArgs)
    {
        TComponent* newComponent = new TComponent(this, std::forward<TArgs>(constructionArgs)...);
        AttachComponent(newComponent);
        return newComponent;
    }

    void DeleteComponent(GameObjectComponent* component);
    void DeleteComponentByID(GameObjectComponentID componentID);

    template<typename TComponent>
    inline void DeleteComponent()
    {
        GameObjectComponent* componentWithType = nullptr;
        for (GameObjectComponent* currComponent: mComponents)
        {
            if (cxx::rtti_cast<TComponent>(currComponent))
            {
                componentWithType = currComponent;
                break;
            }
        }
        if (componentWithType)
        {
            DeleteComponent(currComponent);
        }
    }

    // iterate all gameobject components    
    template<typename TProc>
    inline void ForEachComponent(TProc proc) const
    {
        for (GameObjectComponent* currComponent: mComponents)
        {
            proc(currComponent);
        }
    }

    // destroy all components including transform
    void DeleteAllComponents();

    // get component by type
    template<typename TComponent>
    inline TComponent* GetComponent() const
    {
        for (GameObjectComponent* currComponent: mComponents)
        {
            if (TComponent* componentWithType = cxx::rtti_cast<TComponent>(currComponent))
                return componentWithType;
        }
        return nullptr;
    }

    // get gameobject component by its unique identifier
    GameObjectComponent* GetComponentByID(GameObjectComponentID componentID) const;

    // test whether object has specific component
    template<typename TComponent>
    inline bool HasComponent() const
    {
        if (TComponent* componentWithType = GetComponent<TComponent>())
            return true;

        return false;
    }

    bool HasComponentWithID(GameObjectComponentID componentID) const;

private:
    inline GameObjectComponent* GetComponentByRttiType(const cxx::rtti_type* rttiType) const
    {
        for (GameObjectComponent* currComponent: mComponents)
        {
            if (rttiType == currComponent->get_rtti_type())
                return currComponent;
        }
        return nullptr;
    }

    void AttachComponent(GameObjectComponent* component);
    void UpdateComponentsCache();

private:
    std::vector<GameObjectComponent*> mComponents;
};