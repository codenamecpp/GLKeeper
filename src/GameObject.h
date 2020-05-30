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
    bool AddComponent(GameObjectComponent* component);
    void DeleteComponent(GameObjectComponent* component);

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

    // test whether object has specific component
    template<typename TComponent>
    inline bool HasComponent() const
    {
        if (TComponent* componentWithType = GetComponent<TComponent>())
            return true;

        return false;
    }

    // test whether game object is currently attached to scene and therefore may be rendered
    bool IsAttachedToScene() const;
    void SetAttachedToScene(bool isAttached);

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

    void UpdateComponentsCache();

private:
    std::vector<GameObjectComponent*> mComponents;
    bool mIsAttachedToScene;
};