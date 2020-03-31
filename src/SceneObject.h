#pragma once

#include "SceneDefs.h"
#include "RenderDefs.h"
#include "SceneObjectComponent.h"
#include "AnimatingModelComponent.h"
#include "SceneObjectTransform.h"
#include "TerrainMeshComponent.h"
#include "WaterLavaMeshComponent.h"

class SceneObject: public cxx::noncopyable
{
public:
    SceneObjectComponent* mComponents[eSceneObjectComponent_Count];
    Color32 mDebugColor; // color used for debug draw

    float mDistanceToCameraSquared; // this value gets updated during scene rendition

public:
    SceneObject();
    ~SceneObject();

    // process scene update frame
    // @param deltaTime: Time since last update
    void UpdateFrame(float deltaTime);

    // add or remove object component, object takes ownership on pointer
    // @param component: Component instance
    void AddComponent(SceneObjectComponent* component);
    void DeleteComponent(SceneObjectComponent* component);
    void DeleteComponent(eSceneObjectComponent componentId);

    // destroy all components including transform
    void DeleteAllComponents();
    void DeleteRenderableComponents();

    // test whether object has specific component
    bool HasComponent(eSceneObjectComponent componentId) const;

    // get attached animating model component
    // @returns null if object does not have component
    inline AnimatingModelComponent* GetAnimatingModelComponent() const
    {
        return GetComponent<AnimatingModelComponent>(eSceneObjectComponent_AnimatingModel);
    }
    // get attached transform and bounds component
    // @returns null if object does not have component
    inline SceneObjectTransform* GetTransformComponent() const
    {
        return GetComponent<SceneObjectTransform>(eSceneObjectComponent_Transform);
    }
    // get attached terrain mesh component
    // @returns null if object does not have component
    inline TerrainMeshComponent* GetTerrainMeshComponent() const
    {
        return GetComponent<TerrainMeshComponent>(eSceneObjectComponent_TerrainMesh);
    }
    // get attached water or lava mesh component
    // @returns null if object does not have component
    inline WaterLavaMeshComponent* GetWaterLavaMeshComponent() const
    {
        return GetComponent<WaterLavaMeshComponent>(eSceneObjectComponent_WaterLavaMesh);
    }

    // test whether scene entity is currently attached to scene and therefore may be rendered
    bool IsAttachedToScene() const;
    void SetAttachedToScene(bool isAttached);

private:

    template<typename TObjectComponent>
    inline TObjectComponent* GetComponent(eSceneObjectComponent componentId) const
    {
        return static_cast<TObjectComponent*>(mComponents[componentId]);
    }

private:
    bool mIsAttachedToScene;
};