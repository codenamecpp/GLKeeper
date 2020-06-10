#pragma once

#include "EntityComponent.h"
#include "GameDefs.h"

class Entity: public cxx::noncopyable
{
public:
    Color32 mDebugColor; // color used for debug draw

    float mDistanceToCameraSquared; // this value gets updated during scene rendition

    // components cache
    TransformComponent* mTransform = nullptr;
    RenderableComponent* mRenderable = nullptr;

public:
    Entity();
    ~Entity();

    // process update frame
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

    void DeleteComponent(EntityComponent* component);

    template<typename TComponent>
    inline void DeleteComponent()
    {
        EntityComponent* componentWithType = nullptr;
        for (EntityComponent* currComponent: mComponents)
        {
            if (cxx::rtti_cast<TComponent>(currComponent))
            {
                componentWithType = currComponent;
                break;
            }
        }
        if (componentWithType)
        {
            DeleteComponent(componentWithType);
        }
    }

    // iterate all components    
    template<typename TProc>
    inline void ForEachComponent(TProc proc) const
    {
        for (EntityComponent* currComponent: mComponents)
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
        for (EntityComponent* currComponent: mComponents)
        {
            if (TComponent* componentWithType = cxx::rtti_cast<TComponent>(currComponent))
                return componentWithType;
        }
        return nullptr;
    }

    // test whether object has specific component
    template<typename TComponent>
    inline bool HasComponent() const
    {
        if (TComponent* componentWithType = GetComponent<TComponent>())
            return true;

        return false;
    }

    // enable or disable all attached component within entity
    void SetActive(bool isActive);

    // test whether entity has at least one component which is in enabled state
    bool IsActive() const;

private:
    inline EntityComponent* GetComponentByRttiType(const cxx::rtti_type* rttiType) const
    {
        for (EntityComponent* currComponent: mComponents)
        {
            if (rttiType == currComponent->get_rtti_type())
                return currComponent;
        }
        return nullptr;
    }

    void AttachComponent(EntityComponent* component);
    void UpdateComponentsCache();

private:
    std::vector<EntityComponent*> mComponents;
};