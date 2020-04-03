#include "pch.h"
#include "GameObjectComponentsFactory.h"

GameObjectComponentsFactory gComponentsFactory;

GameObjectComponent* GameObjectComponentsFactory::CreateComponent(GameObject* gameObject, const std::string& componentType)
{
    eGameObjectComponent componentTypeId;
    if (cxx::parse_enum(componentType.c_str(), componentTypeId))
    {
        return CreateComponent(gameObject, componentTypeId);
    }
    debug_assert(false);
    return nullptr;
}

GameObjectComponent* GameObjectComponentsFactory::CreateComponent(GameObject* gameObject, eGameObjectComponent componentType)
{
    switch (componentType)
    {
        case eGameObjectComponent_Transform: return CreateTransformComponent(gameObject);
        case eGameObjectComponent_TerrainMesh: return CreateTerrainMeshComponent(gameObject);
        case eGameObjectComponent_WaterLavaMesh: return CreateWaterLavaMeshComponent(gameObject);
        case eGameObjectComponent_AnimatingModel: return CreateAnimModelComponent(gameObject);
    }
    debug_assert(false);
    return nullptr;
}

TransformComponent* GameObjectComponentsFactory::CreateTransformComponent(GameObject* gameObject)
{
    debug_assert(gameObject);

    return mTransformComponentsPool.create(gameObject);
}

AnimModelComponent* GameObjectComponentsFactory::CreateAnimModelComponent(GameObject* gameObject)
{
    debug_assert(gameObject);

    return mAnimModelComponentsPool.create(gameObject);
}

TerrainMeshComponent* GameObjectComponentsFactory::CreateTerrainMeshComponent(GameObject* gameObject)
{
    debug_assert(gameObject);

    return mTerrainMeshComponentsPool.create(gameObject);
}

WaterLavaMeshComponent* GameObjectComponentsFactory::CreateWaterLavaMeshComponent(GameObject* gameObject)
{
    debug_assert(gameObject);

    return mWaterLavaMeshComponentsPool.create(gameObject);
}

void GameObjectComponentsFactory::DestroyComponent(GameObjectComponent* component)
{
    debug_assert(component);

    if (component)
    {
        switch (component->mComponentType)
        {
            case eGameObjectComponent_Transform: 
            {
                TransformComponent* transformComponent = static_cast<TransformComponent*> (component);
                mTransformComponentsPool.destroy(transformComponent);
            }
            break;

            case eGameObjectComponent_TerrainMesh:
            {
                TerrainMeshComponent* terrainMeshComponent = static_cast<TerrainMeshComponent*> (component);
                mTerrainMeshComponentsPool.destroy(terrainMeshComponent);
            }
            break;

            case eGameObjectComponent_WaterLavaMesh:
            {
                WaterLavaMeshComponent* waterLavaMeshComponent = static_cast<WaterLavaMeshComponent*> (component);
                mWaterLavaMeshComponentsPool.destroy(waterLavaMeshComponent);
            }
            break;

            case eGameObjectComponent_AnimatingModel:
            {
                AnimModelComponent* animModelComponent = static_cast<AnimModelComponent*> (component);
                mAnimModelComponentsPool.destroy(animModelComponent);
            }
            break;

            default:
                debug_assert(false);
            break;
        }
    }

}
