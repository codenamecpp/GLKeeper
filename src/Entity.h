#pragma once

#include "EntityComponent.h"
#include "GameDefs.h"
#include "SceneDefs.h"
#include "EntityController.h"

// scene entity is components container
class Entity final: public cxx::noncopyable
{
public:
    Color32 mDebugColor; // color used for debug draw

    float mDistanceToCameraSquared; // this value gets updated during scene rendition

    // readonly

    // components cache
    TransformComponent* mTransform = nullptr;
    RenderableComponent* mRenderable = nullptr;

    // entity owner can control and listen entity components
    EntityController* mController = nullptr;

public:
    Entity();
    ~Entity();

    // update active entity components
    // @param deltaTime: Time since last update
    void UpdateComponents(float deltaTime);
    
    // assign new or clear current controller if nullptr specified
    void SetController(EntityController* controller);

    // add or remove object component, object takes ownership on pointer
    // @param component: Component instance
    template<typename TComponent, typename... TArgs>
    inline TComponent* AddComponent(const TArgs &&...  constructionArgs)
    {
        TComponent* newComponent = new TComponent(this, std::forward<TArgs>(constructionArgs)...);
        AttachComponent(newComponent);
        return newComponent;
    }

    // immediately destroy specific component
    void RemoveComponent(EntityComponent* component);

    // immediately destroy component of specific type
    template<typename TComponent>
    inline void RemoveComponent()
    {
        TComponent* componentWithType = nullptr;
        for (EntityComponent* currComponent: mComponents)
        {
            componentWithType = entity_component_cast<TComponent>(currComponent);
            if (componentWithType)
                break;
        }
        if (componentWithType)
        {
            RemoveComponent(componentWithType);
        }
    }

    // immediately destroy all components including transform
    void RemoveAllComponents();

    // iterate all components    
    template<typename TProc>
    inline void ForEachComponent(TProc proc) const
    {
        for (EntityComponent* currComponent: mComponents)
        {
            proc(currComponent);
        }
    }

    // get component by type
    template<typename TComponent>
    inline TComponent* GetComponent() const
    {
        for (EntityComponent* currComponent: mComponents)
        {
            TComponent* componentWithType = entity_component_cast<TComponent>(currComponent);
            if (componentWithType)
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