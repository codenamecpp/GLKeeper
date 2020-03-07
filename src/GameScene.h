#pragma once

#include "SceneDefs.h"
#include "SceneCamera.h"
#include "AABBTree.h"

// manages scene objects visualization and transformation
class GameScene: public cxx::noncopyable
{
public:
    SceneCamera mCamera; // main gamescene camera

public:

    // setup internal scene resources
    bool Initialize();
    void Deinit();

    // process single frame logic
    void UpdateFrame();

    // create scene object but not attach it automatically
    // @param position: Position on scene
    // @param direction: Direction vector, must be normalized
    // @param scaling: Scaling
    SceneObject3D* CreateSceneObject(const glm::vec3& position, const glm::vec3& direction, float scaling);
    SceneObject3D* CreateSceneObject();

    // attach entity to scene - attached entity will be rendered and updated, 
    // it is recommended to set transformation and bounding volume before attach
    // @param sceneObject: Target
    void AttachSceneEntity(SceneObject3D* sceneObject);
    void DetachSceneEntity(SceneObject3D* sceneObject);

    // destroy scene entity and free it internal resources, reference become invalid
    // @param sceneEntity: Target
    void DestroySceneObject(SceneObject3D* sceneObject);

    // callback from scene entities
    // Transformation or local bounds of object gets changed
    void HandleSceneObjectTransformChange(SceneObject3D* sceneObject);

private:
    void BuildAABBTree();
    void DestroyAttachedSceneObjects();

private:
    AABBTree mAABBTree;
    cxx::object_pool<SceneObject3D> mObjectsPool;

    // entities lists
    cxx::intrusive_list<SceneObject3D> mTransformEntities;
    cxx::intrusive_list<SceneObject3D> mSceneEntities;

};

extern GameScene gGameScene;