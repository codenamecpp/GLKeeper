#include "pch.h"
#include "RenderScene.h"
#include "Entity.h"
#include "ConsoleVariable.h"
#include "Console.h"
#include "CameraController.h"
#include "TimeManager.h"
#include "DebugRenderer.h"
#include "SceneRenderList.h"
#include "TexturesManager.h"
#include "RenderManager.h"
#include "TransformComponent.h"

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

    DetachEntities();
    mAABBTree.Cleanup();
}

void RenderScene::UpdateFrame()
{
    float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();

    if (mCameraController)
    {
        mCameraController->HandleUpdateFrame(deltaTime);
    }

    BuildAABBTree();
}

void RenderScene::CollectObjectsForRendering()
{
    mCamera.ComputeMatrices();
    mAABBTree.QueryEntities(mCamera.mFrustum, [this](Entity* sceneObject)
    {
        gRenderManager.RegisterEntityForRendering(sceneObject);
        // update distance to camera
        TransformComponent* transformComponent = sceneObject->mTransform; 
        sceneObject->mDistanceToCameraSquared = glm::length2(transformComponent->mPosition - mCamera.mPosition);
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

void RenderScene::HandleTransformChange(Entity* sceneEntity)
{
    debug_assert(sceneEntity);
    cxx::push_back_if_unique(mTransformObjects, sceneEntity); // queue for update
}

void RenderScene::AttachEntity(Entity* sceneEntity)
{
    debug_assert(sceneEntity);
    // already attached to scene
    if (cxx::contains(mSceneObjects, sceneEntity))
    {
        debug_assert(false);
        return;
    }
    mSceneObjects.push_back(sceneEntity);

    // refresh aabbtree node
    mAABBTree.InsertEntity(sceneEntity);
}

void RenderScene::DetachEntity(Entity* sceneEntity)
{
    debug_assert(sceneEntity);
    if (cxx::contains(mSceneObjects, sceneEntity))
    {
        cxx::erase_elements(mSceneObjects, sceneEntity);
        cxx::erase_elements(mTransformObjects, sceneEntity);
    }
    else
    {
        return; // already detached
    }

    // refresh aabbtree node
    mAABBTree.RemoveEntity(sceneEntity);
}

void RenderScene::DetachEntities()
{
    while (mSceneObjects.size())
    {
        Entity* entity = mSceneObjects.back();
        DetachEntity(entity);
    }

    debug_assert(mTransformObjects.empty());
    mTransformObjects.clear();
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
        Entity* sceneObject = mTransformObjects.back();
        mTransformObjects.pop_back();

        // refresh aabbtree node
        mAABBTree.UpdateEntity(sceneObject);
    }
}
