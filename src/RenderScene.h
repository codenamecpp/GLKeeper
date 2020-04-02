#pragma once

#include "SceneCamera.h"
#include "AABBTree.h"

class RenderScene: public cxx::noncopyable
{
public:
    SceneCamera mCamera; // main gamescene camera

public:

    // setup internal scene resources
    bool Initialize();
    void Deinit();

    // process single frame logic
    void UpdateFrame();

    // collect all visible scene objects
    void CollectObjectsForRendering();

    // process debug draw
    void DebugRenderFrame(DebugRenderer& renderer);

    // process input event
    // @param inputEvent: Event data
    void HandleInputEvent(MouseButtonInputEvent& inputEvent);
    void HandleInputEvent(MouseMovedInputEvent& inputEvent);
    void HandleInputEvent(MouseScrollInputEvent& inputEvent);
    void HandleInputEvent(KeyInputEvent& inputEvent);
    void HandleInputEvent(KeyCharEvent& inputEvent);

    // attach renderable game object to scene - it will be rendered 
    // it is recommended to set transformation and bounding volume before attach
    // @param sceneObject: Target
    void AttachObject(GameObject* sceneObject);
    void DetachObject(GameObject* sceneObject);
    void DetachObjects();

    // set active scene camera controller
    // @param cameraController: Contoller or null to clear current
    void SetCameraController(CameraController* cameraController);

    // callback from scene entities
    // Transformation or local bounds of object gets changed
    void HandleTransformChange(GameObject* sceneObject);

private:
    void BuildAABBTree();

private:
    AABBTree mAABBTree;
    CameraController* mCameraController = nullptr;
    // entities lists
    std::vector<GameObject*> mTransformObjects;
    std::vector<GameObject*> mSceneObjects;
};

extern RenderScene gRenderScene;