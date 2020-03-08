#include "pch.h"
#include "GameScene.h"
#include "SceneObject.h"
#include "ConsoleVariable.h"
#include "Console.h"
#include "CameraControl.h"
#include "TimeManager.h"

//////////////////////////////////////////////////////////////////////////

// cvars
CvarBoolean gCvarScene_DebugDrawAabb ( "dbg_drawSceneAabb", true, "Draw scene aabb debug data", ConsoleVar_Debug | ConsoleVar_Scene );

//////////////////////////////////////////////////////////////////////////

GameScene gGameScene;

//////////////////////////////////////////////////////////////////////////

bool GameScene::Initialize()
{
    gConsole.RegisterVariable(&gCvarScene_DebugDrawAabb);
    return true;
}

void GameScene::Deinit()
{
    mCameraControl = nullptr;

    gConsole.UnregisterVariable(&gCvarScene_DebugDrawAabb);
    mAABBTree.Cleanup();

    DestroyAttachedSceneObjects();
}

void GameScene::UpdateFrame()
{
    float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();

    if (mCameraControl)
    {
        mCameraControl->HandleUpdateFrame(deltaTime);
    }

    BuildAABBTree();
}

void GameScene::DebugRenderFrame(DebugRenderer& renderer)
{
    if (gCvarScene_DebugDrawAabb.mValue)
    {
        mAABBTree.DebugRender(renderer);
    }
}

void GameScene::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mCameraControl)
    {
        mCameraControl->HandleInputEvent(inputEvent);
    }
}

void GameScene::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCameraControl)
    {
        mCameraControl->HandleInputEvent(inputEvent);
    }
}

void GameScene::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCameraControl)
    {
        mCameraControl->HandleInputEvent(inputEvent);
    }
}

void GameScene::HandleInputEvent(KeyInputEvent& inputEvent)
{
    if (mCameraControl)
    {
        mCameraControl->HandleInputEvent(inputEvent);
    }
}

void GameScene::HandleInputEvent(KeyCharEvent& inputEvent)
{
    if (mCameraControl)
    {
        mCameraControl->HandleInputEvent(inputEvent);
    }
}

SceneObject* GameScene::CreateSceneObject(const glm::vec3& position, const glm::vec3& direction, float scaling)
{
    SceneObject* sceneEntity = mObjectsPool.create();
    sceneEntity->SetPositionOnScene(position);
    // todo : direction
    sceneEntity->SetScaling(scaling);
    return sceneEntity;
}

SceneObject* GameScene::CreateSceneObject()
{
    SceneObject* sceneEntity = mObjectsPool.create();
    return sceneEntity;
}

void GameScene::HandleSceneObjectTransformChange(SceneObject* sceneEntity)
{
    debug_assert(sceneEntity);
    if (!mTransformObjects.contains(&sceneEntity->mListNodeTransformed))
    {
        mTransformObjects.insert(&sceneEntity->mListNodeTransformed); // queue for update
    }
}

void GameScene::AttachSceneObject(SceneObject* sceneEntity)
{
    debug_assert(sceneEntity);
    // already attached to scene
    if (mSceneObjects.contains(&sceneEntity->mListNodeAttached))
    {
        debug_assert(false);
        return;
    }
    else
    {
        mSceneObjects.insert(&sceneEntity->mListNodeAttached);
    }

    mAABBTree.InsertObject(sceneEntity);
}

void GameScene::DetachSceneObject(SceneObject* sceneEntity)
{
    debug_assert(sceneEntity);

    if (mSceneObjects.contains(&sceneEntity->mListNodeAttached))
    {
        mSceneObjects.remove(&sceneEntity->mListNodeAttached);
    }
    else
    {
        return; // already detached
    }

    if (mTransformObjects.contains(&sceneEntity->mListNodeTransformed))
    {
        mTransformObjects.remove(&sceneEntity->mListNodeTransformed);
    }

    mAABBTree.RemoveObject(sceneEntity);
}

void GameScene::DestroySceneObject(SceneObject* sceneEntity)
{
    debug_assert(sceneEntity);

    DetachSceneObject(sceneEntity);
    mObjectsPool.destroy(sceneEntity);
}

void GameScene::SetCameraControl(SceneCameraControl* cameraController)
{
    if (mCameraControl == cameraController)
        return;

    if (mCameraControl)
    {
        mCameraControl->HandleSceneDetach();
        mCameraControl->mSceneCamera = nullptr;
    }
    mCameraControl = cameraController;
    if (mCameraControl)
    {
        mCameraControl->mSceneCamera = &mCamera;
        mCameraControl->HandleSceneDetach();
    }
}

void GameScene::BuildAABBTree()
{
    while (mTransformObjects.has_elements())
    {
        cxx::intrusive_node<SceneObject>* entityNode = mTransformObjects.get_head_node();
        mTransformObjects.remove(entityNode);

        // refresh aabbtree node
        SceneObject* sceneEntity = entityNode->get_element();
        mAABBTree.UpdateObject(sceneEntity);
    }
}

void GameScene::DestroyAttachedSceneObjects()
{
    while (mTransformObjects.has_elements())
    {
        cxx::intrusive_node<SceneObject>* entityNode = mTransformObjects.get_head_node();

        SceneObject* currentEntity = entityNode->get_element();
        DestroySceneObject(currentEntity);
    }

    while (mSceneObjects.has_elements())
    {
        cxx::intrusive_node<SceneObject>* entityNode = mSceneObjects.get_head_node();

        SceneObject* currentEntity = entityNode->get_element();
        DestroySceneObject(currentEntity);
    }
}
