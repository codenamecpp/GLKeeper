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

    // process debug draw
    void DebugRenderFrame(DebugRenderer& renderer);

    // create scene object but not attach it automatically
    // @param position: Position on scene
    // @param direction: Direction vector, must be normalized
    // @param scaling: Scaling
    SceneObject* CreateSceneObject(const glm::vec3& position, const glm::vec3& direction, float scaling);
    SceneObject* CreateSceneObject();

    // attach entity to scene - attached entity will be rendered and updated, 
    // it is recommended to set transformation and bounding volume before attach
    // @param sceneObject: Target
    void AttachSceneObject(SceneObject* sceneObject);
    void DetachSceneObject(SceneObject* sceneObject);

    // destroy scene entity and free it internal resources, reference become invalid
    // @param sceneEntity: Target
    void DestroySceneObject(SceneObject* sceneObject);

    // callback from scene entities
    // Transformation or local bounds of object gets changed
    void HandleSceneObjectTransformChange(SceneObject* sceneObject);

private:
    void BuildAABBTree();
    void DestroyAttachedSceneObjects();

private:
    AABBTree mAABBTree;
    cxx::object_pool<SceneObject> mObjectsPool;

    // entities lists
    cxx::intrusive_list<SceneObject> mTransformObjects;
    cxx::intrusive_list<SceneObject> mSceneObjects;

};

extern GameScene gGameScene;