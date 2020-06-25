#include "pch.h"
#include "RenderScene.h"
#include "SceneObject.h"
#include "ConsoleVariable.h"
#include "Console.h"
#include "CameraController.h"
#include "TimeManager.h"
#include "DebugRenderer.h"
#include "SceneRenderList.h"
#include "TexturesManager.h"
#include "RenderManager.h"

//////////////////////////////////////////////////////////////////////////

// cvars
CvarBoolean gCvarScene_DebugDrawAabb ( "dbg_drawSceneAabb", true, "Draw scene aabb debug data", ConsoleVar_Debug | ConsoleVar_Scene );

//////////////////////////////////////////////////////////////////////////

RenderScene gRenderScene;

//////////////////////////////////////////////////////////////////////////

bool RenderScene::Initialize()
{
    gConsole.RegisterVariable(&gCvarScene_DebugDrawAabb);
    return true;
}

void RenderScene::Deinit()
{
    mCameraController = nullptr;

    gConsole.UnregisterVariable(&gCvarScene_DebugDrawAabb);

    DetachObjects();
    mAABBTree.Cleanup();
}

void RenderScene::DestroyObjects()
{
    while (mSceneObjects.size())
    {
        SceneObject* currObject = mSceneObjects.back();
        currObject->DestroyObject();
    }
}

void RenderScene::UpdateFrame()
{
    float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();

    if (mCameraController)
    {
        mCameraController->HandleUpdateFrame(deltaTime);
    }

    for (SceneObject* currObject: mSceneObjects)
    {
        currObject->UpdateFrame(deltaTime);
    }

    BuildAABBTree();
}

void RenderScene::CollectObjectsForRendering()
{
    mCamera.ComputeMatrices();
    mAABBTree.QueryObjects(mCamera.mFrustum, [this](SceneObject* sceneObject)
    {
        gRenderManager.RegisterObjectForRendering(sceneObject);
        // update distance to camera 
        sceneObject->mDistanceToCameraSquared = glm::length2(sceneObject->mPosition - mCamera.mPosition);
    });
}

void RenderScene::DebugRenderFrame(DebugRenderer& renderer)
{
    if (!gCvarScene_DebugDrawAabb.mValue)
        return;

    renderer.DrawAxes(glm::mat4(1.0f), glm::vec3(0.0f), 2.0f, false);
    mAABBTree.DebugRender(renderer);
}

void RenderScene::ProcessInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->HandleInputEvent(inputEvent);
    }
}

void RenderScene::ProcessInputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->HandleInputEvent(inputEvent);
    }
}

void RenderScene::ProcessInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->HandleInputEvent(inputEvent);
    }
}

void RenderScene::ProcessInputEvent(KeyInputEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->HandleInputEvent(inputEvent);
    }
}

void RenderScene::ProcessInputEvent(KeyCharEvent& inputEvent)
{
    if (mCameraController)
    {
        mCameraController->HandleInputEvent(inputEvent);
    }
}

void RenderScene::HandleTransformChange(SceneObject* sceneObject)
{
    debug_assert(sceneObject);
    cxx::push_back_if_unique(mTransformObjects, sceneObject); // queue for update
}

void RenderScene::AttachObject(SceneObject* sceneObject)
{
    debug_assert(sceneObject);
    // already attached to scene
    if (cxx::contains(mSceneObjects, sceneObject))
    {
        debug_assert(false);
        return;
    }
    mSceneObjects.push_back(sceneObject);

    // refresh aabbtree node
    mAABBTree.InsertObject(sceneObject);
}

void RenderScene::DetachObject(SceneObject* sceneObject)
{
    debug_assert(sceneObject);
    if (cxx::contains(mSceneObjects, sceneObject))
    {
        cxx::erase_elements(mSceneObjects, sceneObject);
        cxx::erase_elements(mTransformObjects, sceneObject);
    }
    else
    {
        return; // already detached
    }

    // refresh aabbtree node
    mAABBTree.RemoveObject(sceneObject);
}

void RenderScene::DetachObjects()
{
    while (mSceneObjects.size())
    {
        SceneObject* sceneObject = mSceneObjects.back();
        DetachObject(sceneObject);
    }

    debug_assert(mTransformObjects.empty());
    mTransformObjects.clear();
}

bool RenderScene::IsObjectAttached(const SceneObject* sceneObject) const
{
    debug_assert(sceneObject);

    return cxx::contains(mSceneObjects, sceneObject);
}

void RenderScene::SetCameraController(CameraController* cameraController)
{
    if (mCameraController == cameraController)
        return;

    if (mCameraController)
    {
        mCameraController->HandleSceneDetach();
        mCameraController->mSceneCamera = nullptr;
    }
    mCameraController = cameraController;
    if (mCameraController)
    {
        mCameraController->mSceneCamera = &mCamera;
        mCameraController->HandleSceneAttach();
    }
}

void RenderScene::BuildAABBTree()
{
    while (!mTransformObjects.empty())
    {
        SceneObject* sceneObject = mTransformObjects.back();
        mTransformObjects.pop_back();

        // refresh aabbtree node
        mAABBTree.UpdateObject(sceneObject);
    }
}
