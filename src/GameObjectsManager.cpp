#include "pch.h"
#include "GameObjectsManager.h"
#include "GameObject.h"
#include "TimeManager.h"
#include "GameObjectComponentsFactory.h"

GameObjectsManager gGameObjectsManager;

bool GameObjectsManager::Initialize()
{
    return true;
}

void GameObjectsManager::Deinit()
{
}

void GameObjectsManager::EnterWorld()
{
    mIDsCounter = 0;
}

void GameObjectsManager::ClearWorld()
{
    DestroyObjectsList();
}

void GameObjectsManager::UpdateFrame()
{
    float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();

    bool doCleanup = false;

    // iterating by index here - game objects could be added duting update frame
    for (size_t icurrent = 0, Count = mObjectsList.size(); 
        icurrent < Count; ++icurrent)
    {
        GameObject* currObject = mObjectsList[icurrent];
        if (currObject == nullptr)
        {
            doCleanup = true;
            continue;
        }
        currObject->UpdateFrame(deltaTime);
    }

    if (doCleanup)
    {
        cxx::erase_elements(mObjectsList, nullptr);
    }
}

void GameObjectsManager::DestroyGameObject(GameObject* gameObject)
{
    debug_assert(gameObject);

    for (size_t icurrent = 0, Count = mObjectsList.size(); 
        icurrent < Count; ++icurrent)
    {
        if (mObjectsList[icurrent] == gameObject)
        {
            mObjectsList[icurrent] = nullptr;
            break;
        }
    }

    mGameObjectsPool.destroy(gameObject);
}

GameObjectInstanceID GameObjectsManager::GenerateNewInstanceID()
{
    // todo
    return ++mIDsCounter;
}

void GameObjectsManager::DestroyObjectsList()
{
    for (GameObject* currObject: mObjectsList)
    {
        if (currObject)
        {
            mGameObjectsPool.destroy(currObject);
        }
    }

    mObjectsList.clear();
}

GameObject* GameObjectsManager::CreateGameObject(const glm::vec3& position, const glm::vec3& direction, float scaling)
{
    GameObject* gameObject = CreateGameObject();
    debug_assert(gameObject);

    TransformComponent* transformComponent = gameObject->GetTransformComponent();
    debug_assert(transformComponent);

    transformComponent->SetPosition(position);
    transformComponent->SetScaling(scaling);

    return gameObject;
}

GameObject* GameObjectsManager::CreateGameObject()
{
    GameObjectInstanceID instanceID = GenerateNewInstanceID();

    GameObject* gameObject = mGameObjectsPool.create(instanceID);
    mObjectsList.push_back(gameObject);

    TransformComponent* transformComponent = gComponentsFactory.CreateTransformComponent(gameObject);
    gameObject->AddComponent(transformComponent);

    return gameObject;
}
