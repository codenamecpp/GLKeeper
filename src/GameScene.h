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

    // attach entity to scene - attached entity will be rendered and updated, 
    // it is recommended to set transformation and bounding volume before attach
    // @param sceneObject: Target
    void AttachSceneEntity(SceneObject3D* sceneObject);
    void DetachSceneEntity(SceneObject3D* sceneObject);

    // destroy scene entity and free it internal resources, reference become invalid
    // @param sceneEntity: Target
    void ReleaseSceneObject(SceneObject3D* sceneObject);

    // callback from scene entities
    // Transformation or local bounds of object gets changed
    void HandleSceneObjectTransformChange(SceneObject3D* sceneObject);

private:
    void BuildAABBTree();

private:
    AABBTree mAABBTree;    

    // entities lists
    cxx::intrusive_list<SceneObject3D> mTransformEntities;
    cxx::intrusive_list<SceneObject3D> mSceneEntities;

};

extern GameScene gGameScene;