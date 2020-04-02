#include "pch.h"
#include "RenderScene.h"
#include "GameObject.h"
#include "ConsoleVariable.h"
#include "Console.h"
#include "CameraControl.h"
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
    mCameraControl = nullptr;

    gConsole.UnregisterVariable(&gCvarScene_DebugDrawAabb);

    DetachObjects();
    mAABBTree.Cleanup();
}

void RenderScene::UpdateFrame()
{
    float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();

    if (mCameraControl)
    {
        mCameraControl->HandleUpdateFrame(deltaTime);
    }

    BuildAABBTree();
}

void RenderScene::CollectObjectsForRendering()
{
    mCamera.ComputeMatrices();
    mAABBTree.QueryObjects(mCamera.mFrustum, [this](GameObject* sceneObject)
    {
        gRenderManager.RegisterSceneObjectForRendering(sceneObject);

        TransformComponent* transformComponent = sceneObject->GetTransformComponent();
        // update distance to camera
        sceneObject->mDistanceToCameraSquared = glm::length2(transformComponent->mPosition - mCamera.mPosition);
    });
}

void RenderScene::DebugRenderFrame(DebugRenderer& renderer)
{
    if (!gCvarScene_DebugDrawAabb.mValue)
        return;

    Size2D cellCount (12, 12);
    glm::vec2 cellSize (1.0f, 1.0f);
    renderer.DrawGrid(cellCount, cellSize, Color32_Gray, true);
    renderer.DrawAxes(glm::mat4(1.0f), glm::vec3(0.0f), 2.0f, false);
    mAABBTree.DebugRender(renderer);
}

void RenderScene::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mCameraControl)
    {
        mCameraControl->HandleInputEvent(inputEvent);
    }
}

void RenderScene::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mCameraControl)
    {
        mCameraControl->HandleInputEvent(inputEvent);
    }
}

void RenderScene::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mCameraControl)
    {
        mCameraControl->HandleInputEvent(inputEvent);
    }
}

void RenderScene::HandleInputEvent(KeyInputEvent& inputEvent)
{
    if (mCameraControl)
    {
        mCameraControl->HandleInputEvent(inputEvent);
    }
}

void RenderScene::HandleInputEvent(KeyCharEvent& inputEvent)
{
    if (mCameraControl)
    {
        mCameraControl->HandleInputEvent(inputEvent);
    }
}

void RenderScene::HandleTransformChange(GameObject* sceneEntity)
{
    debug_assert(sceneEntity);
    cxx::push_back_if_unique(mTransformObjects, sceneEntity); // queue for update
}

void RenderScene::AttachObject(GameObject* sceneEntity)
{
    debug_assert(sceneEntity);
    // already attached to scene
    if (sceneEntity->IsAttachedToScene())
    {
        debug_assert(false);
        return;
    }

    sceneEntity->SetAttachedToScene(true);
    mSceneObjects.push_back(sceneEntity);

    // refresh aabbtree node
    mAABBTree.InsertObject(sceneEntity);
}

void RenderScene::DetachObject(GameObject* sceneEntity)
{
    debug_assert(sceneEntity);
    if (sceneEntity->IsAttachedToScene())
    {
        sceneEntity->SetAttachedToScene(false);

        cxx::erase_elements(mSceneObjects, sceneEntity);
        cxx::erase_elements(mTransformObjects, sceneEntity);
    }
    else
    {
        return; // already detached
    }

    // refresh aabbtree node
    mAABBTree.RemoveObject(sceneEntity);
}

void RenderScene::DetachObjects()
{
    while (mSceneObjects.size())
    {
        GameObject* gameObject = mSceneObjects.back();
        DetachObject(gameObject);
    }

    debug_assert(mTransformObjects.empty());
    mTransformObjects.clear();
}

void RenderScene::SetCameraControl(SceneCameraControl* cameraController)
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
        mCameraControl->HandleSceneAttach();
    }
}

void RenderScene::BuildAABBTree()
{
    while (!mTransformObjects.empty())
    {
        GameObject* sceneObject = mTransformObjects.back();
        mTransformObjects.pop_back();

        // refresh aabbtree node
        mAABBTree.UpdateObject(sceneObject);
    }
}
