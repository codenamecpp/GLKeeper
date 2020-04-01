#pragma once

#include "GameDefs.h"

class GameObjectsManager: public cxx::noncopyable
{
public:
    // readonly
    std::vector<GameObject*> mObjectsList;

public:

    bool Initialize();
    void Deinit();

    void EnterWorld();
    void ClearWorld();
    void UpdateFrame();

    // @param position: Position on scene
    // @param direction: Direction vector, must be normalized
    // @param scaling: Scaling
    GameObject* CreateGameObject(const glm::vec3& position, const glm::vec3& direction, float scaling);
    GameObject* CreateGameObject();

    // destroy game object immediately, pointer becomes invalid
    void DestroyGameObject(GameObject* gameObject);

private:
    GameObjectInstanceID GenerateNewInstanceID();

    void DestroyObjectsList();

private:
    GameObjectInstanceID mIDsCounter = 0;
};

extern GameObjectsManager gGameObjectsManager;