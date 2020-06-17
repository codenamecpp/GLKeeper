#pragma once

#include "GameDefs.h"

// known object class names
#define OBJECT_NAME_DUNGEON_HEART "Dungeon Heart"
#define OBJECT_NAME_TEMPLE_HAND "Temple Hand"
#define OBJECT_NAME_FRONTEND_BANNER1 "3D Front End Banner 1"
#define OBJECT_NAME_FRONTEND_BANNER2 "3D Front End Banner 2"
#define OBJECT_NAME_FRONTEND_BANNER3 "3D Front End Banner 3"
#define OBJECT_NAME_FRONTEND_BANNER4 "3D Front End Banner 4"
#define OBJECT_NAME_FRONTEND_GEM_HOLDER "3D Front End Gem Holder"
#define OBJECT_NAME_FRONTEND_CHAIN "3D Front End Chain"

// game objects manager
class GameObjectsManager: public cxx::noncopyable
{
public:
    // readonly
    std::vector<GameObject*> mGameObjectsList;

public:
    // one time initialization/shutdown routine
    bool Initialize();
    void Deinit();

    void EnterWorld();
    void ClearWorld();
    void UpdateFrame();

    // @param typeID: Type identifier
    // @param position: Position on scene
    // @param direction: Direction vector, must be normalized
    // @param scaling: Scaling
    GameObject* CreateObject(GameObjectTypeID typeID, const glm::vec3& position, const glm::vec3& direction, float scaling);
    GameObject* CreateObject(GameObjectTypeID typeID);
    // @param definition: Type definition
    GameObject* CreateObject(GameObjectDefinition* definition, const glm::vec3& position, const glm::vec3& direction, float scaling);
    GameObject* CreateObject(GameObjectDefinition* definition);

    // destroy gameobject immediately, pointer becomes invalid
    void DestroyGameObject(GameObject* gameObject);

private:
    void DestroyGameObjects();

    // generate unqiue identifier
    GameObjectID NextUniqueID();

private:
    
};

extern GameObjectsManager gGameObjectsManager;