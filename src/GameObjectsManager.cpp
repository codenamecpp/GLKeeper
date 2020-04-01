#include "pch.h"
#include "GameObjectsManager.h"
#include "GameObject.h"
#include "TimeManager.h"

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
    float deltaTime = gTimeManager.GetRealtimeFrameDelta();

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

    SafeDelete(gameObject);
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
        SafeDelete(currObject);
    }

    mObjectsList.clear();
}

GameObject* GameObjectsManager::CreateGameObject(const glm::vec3& position, const glm::vec3& direction, float scaling)
{
    GameObject* gameObject = CreateGameObject();
    debug_assert(gameObject);

    GameObjectTransform* transformComponent = gameObject->GetTransformComponent();
    debug_assert(transformComponent);

    transformComponent->SetPosition(position);
    transformComponent->SetScaling(scaling);

    return gameObject;
}

GameObject* GameObjectsManager::CreateGameObject()
{
    GameObjectInstanceID instanceID = GenerateNewInstanceID();

    GameObject* gameObject = new GameObject(instanceID);
    mObjectsList.push_back(gameObject);

    GameObjectTransform* transformComponent = new GameObjectTransform(gameObject);
    gameObject->AddComponent(transformComponent);

    return gameObject;
}
