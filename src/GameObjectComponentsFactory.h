#pragma once

#include "AnimModelComponent.h"
#include "TransformComponent.h"
#include "TerrainMeshComponent.h"
#include "WaterLavaMeshComponent.h"

class GameObjectComponentsFactory: public cxx::noncopyable
{
public:
    // create game object component of specific type
    GameObjectComponent* CreateComponent(GameObject* gameObject, eGameObjectComponent componentType);
    GameObjectComponent* CreateComponent(GameObject* gameObject, const std::string& componentType);
    // @param gameObject: Game object to attach to
    TransformComponent* CreateTransformComponent(GameObject* gameObject);
    AnimModelComponent* CreateAnimModelComponent(GameObject* gameObject);
    TerrainMeshComponent* CreateTerrainMeshComponent(GameObject* gameObject);
    WaterLavaMeshComponent* CreateWaterLavaMeshComponent(GameObject* gameObject);

    // destroy game object component
    // @param component: Component 
    void DestroyComponent(GameObjectComponent* component);

private:
    cxx::object_pool<TransformComponent> mTransformComponentsPool;
    cxx::object_pool<AnimModelComponent> mAnimModelComponentsPool;
    cxx::object_pool<TerrainMeshComponent> mTerrainMeshComponentsPool;
    cxx::object_pool<WaterLavaMeshComponent> mWaterLavaMeshComponentsPool;
};

extern GameObjectComponentsFactory gComponentsFactory;