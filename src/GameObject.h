#pragma once

#include "GameObjectComponent.h"
#include "AnimatingModelComponent.h"
#include "TransformComponent.h"
#include "TerrainMeshComponent.h"
#include "WaterLavaMeshComponent.h"
#include "StaticMeshComponent.h"

class GameObject: public cxx::noncopyable
{
public:
    Color32 mDebugColor; // color used for debug draw

    float mDistanceToCameraSquared; // this value gets updated during scene rendition
    
    // readonly
    GameObjectInstanceID mInstanceID;

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
    void DeleteComponent(eGameObjectComponent componentId);

    // destroy all components including transform
    void DeleteAllComponents();

    // get component
    template<typename TObjectComponent>
    inline TObjectComponent* GetComponent() const
    {
        eGameObjectComponent componentId = TObjectComponent::GetComponentType();
        return static_cast<TObjectComponent*>(mComponents[componentId]);
    }

    // get specific component by its identifier
    inline GameObjectComponent* GetComponent(eGameObjectComponent componentId) const
    {
        debug_assert(componentId < eGameObjectComponent_Count);
        return mComponents[componentId];
    }

    // test whether object has specific component
    // @param componentId: Component type identifier
    bool HasComponent(eGameObjectComponent componentId) const;

    // test whether game object is currently attached to scene and therefore may be rendered
    bool IsAttachedToScene() const;
    void SetAttachedToScene(bool isAttached);

private:
    GameObjectComponent* mComponents[eGameObjectComponent_Count];
    bool mIsAttachedToScene;
};