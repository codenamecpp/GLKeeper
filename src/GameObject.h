#pragma once

#include "GameDefs.h"
#include "GameObjectComponent.h"
#include "AnimatingModel.h"
#include "GameObjectTransform.h"
#include "TerrainMesh.h"
#include "WaterLavaMesh.h"

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
    void DeleteRenderableComponents();

    // test whether object has specific component
    // @param componentId: Component type identifier
    bool HasComponent(eGameObjectComponent componentId) const;
    bool HasUpdatableComponents() const;
    bool HasRenderableComponents() const;

    // get attached animating model component
    // @returns null if object does not have component
    inline AnimatingModel* GetAnimatingModelComponent() const
    {
        return GetComponent<AnimatingModel>(eGameObjectComponent_AnimatingModel);
    }
    // get attached transform and bounds component
    // @returns null if object does not have component
    inline GameObjectTransform* GetTransformComponent() const
    {
        return GetComponent<GameObjectTransform>(eGameObjectComponent_Transform);
    }
    // get attached terrain mesh component
    // @returns null if object does not have component
    inline TerrainMesh* GetTerrainMeshComponent() const
    {
        return GetComponent<TerrainMesh>(eGameObjectComponent_TerrainMesh);
    }
    // get attached water or lava mesh component
    // @returns null if object does not have component
    inline WaterLavaMesh* GetWaterLavaMeshComponent() const
    {
        return GetComponent<WaterLavaMesh>(eGameObjectComponent_WaterLavaMesh);
    }

    // test whether game object is currently attached to scene and therefore may be rendered
    bool IsAttachedToScene() const;
    void SetAttachedToScene(bool isAttached);

private:

    template<typename TObjectComponent>
    inline TObjectComponent* GetComponent(eGameObjectComponent componentId) const
    {
        return static_cast<TObjectComponent*>(mComponents[componentId]);
    }

private:
    GameObjectComponent* mComponents[eGameObjectComponent_Count];
    bool mIsAttachedToScene;
};