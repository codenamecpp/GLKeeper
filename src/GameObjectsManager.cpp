#include "pch.h"
#include "GameObjectsManager.h"
#include "GameWorld.h"
#include "GameObject.h"

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
}

void GameObjectsManager::ClearWorld()
{
    DestroyGameObjects();
}

void GameObjectsManager::UpdateFrame()
{
}

GameObject* GameObjectsManager::CreateObject(GameObjectTypeID typeID, const glm::vec3& position, const glm::vec3& direction, float scaling)
{
    GameObject* gameobject = CreateObject(typeID);
    if (gameobject)
    {
        // todo: set position, direction, scale
    }
    return gameobject;
}

GameObject* GameObjectsManager::CreateObject(GameObjectTypeID typeID)
{
    GameObjectDefinition* objectDefinition = gGameWorld.GetGameObjectDefinition(typeID);
    return CreateObject(objectDefinition);
}

GameObject* GameObjectsManager::CreateObject(GameObjectDefinition* definition, const glm::vec3& position, const glm::vec3& direction, float scaling)
{
    GameObject* gameobject = CreateObject(definition);
    if (gameobject)
    {
        // todo: set position, direction, scale
    }
    return gameobject;
}

GameObject* GameObjectsManager::CreateObject(GameObjectDefinition* definition)
{
    if (definition == nullptr)
    {
        debug_assert(false);
        return false;
    }

    GameObjectID objectID = NextUniqueID();
    GameObject* gameobject = new GameObject(objectID, definition);
    mGameObjectsList.push_back(gameobject);

    gameobject->EnterGameWorld();
    return gameobject;
}

void GameObjectsManager::DestroyGameObject(GameObject* gameObject)
{
    if (gameObject == nullptr)
        return;

    cxx::erase_elements(mGameObjectsList, gameObject);
    gameObject->LeaveGameWorld();

    SafeDelete(gameObject);
}

void GameObjectsManager::DestroyGameObjects()
{
}

GameObjectID GameObjectsManager::NextUniqueID()
{   
    // todo: generate id
    return 0;
}
