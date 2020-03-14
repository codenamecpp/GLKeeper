#include "pch.h"
#include "RenderScene.h"
#include "Renderable.h"
#include "ConsoleVariable.h"
#include "Console.h"
#include "CameraControl.h"
#include "TimeManager.h"
#include "DebugRenderer.h"
#include "AnimatingModel.h"
#include "SceneRenderList.h"

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

    DestroySceneObjects();
    mAABBTree.Cleanup();
}

void RenderScene::UpdateFrame()
{
    float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();

    for (Renderable* currRenderable: mSceneObjects)
    {
        currRenderable->UpdateFrame(deltaTime);
    }

    if (mCameraControl)
    {
        mCameraControl->HandleUpdateFrame(deltaTime);
    }

    BuildAABBTree();
}

void RenderScene::CollectRenderables(SceneRenderList& renderablesList)
{
    mCamera.ComputeMatrices();
    mAABBTree.QueryObjects(mCamera.mFrustum, [&renderablesList, this](Renderable* renderable)
    {
        renderable->RegisterForRendering(renderablesList);
        renderable->mDistanceToCameraSquared = glm::length2(renderable->mPosition - mCamera.mPosition);
    });
}

void RenderScene::DebugRenderFrame(DebugRenderer& renderer)
{
    if (!gCvarScene_DebugDrawAabb.mValue)
        return;

    Size2D cellCount (12, 12);
    glm::vec2 cellSize (1.0f, 1.0f);
    renderer.DrawGrid(cellCount, cellSize, Color32_Gray, true);
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

Renderable* RenderScene::CreateNullRenderable(const glm::vec3& position, const glm::vec3& direction, float scaling)
{
    Renderable* sceneEntity = new Renderable;
    sceneEntity->SetPosition(position);
    // todo : direction
    sceneEntity->SetScaling(scaling);
    return sceneEntity;
}

Renderable* RenderScene::CreateNullRenderable()
{
    Renderable* sceneEntity = new Renderable;
    return sceneEntity;
}

AnimatingModel* RenderScene::CreateAnimatingModel(ModelAsset* modelAsset, const glm::vec3& position, const glm::vec3& direction)
{
    AnimatingModel* sceneEntity = new AnimatingModel;
    sceneEntity->SetPosition(position);
    // todo : direction
    sceneEntity->SetModelAsset(modelAsset);
    return sceneEntity;
}

AnimatingModel* RenderScene::CreateAnimatingModel()
{
    AnimatingModel* sceneEntity = new AnimatingModel;
    return sceneEntity;
}

void RenderScene::HandleTransformChange(Renderable* sceneEntity)
{
    debug_assert(sceneEntity);
    if (!mTransformObjects.contains(&sceneEntity->mListNodeTransformed))
    {
        mTransformObjects.insert(&sceneEntity->mListNodeTransformed); // queue for update
    }
}

void RenderScene::AttachRenderable(Renderable* sceneEntity)
{
    debug_assert(sceneEntity);
    // already attached to scene
    if (mSceneObjects.contains(&sceneEntity->mListNodeOnScene))
    {
        debug_assert(false);
        return;
    }
    else
    {
        mSceneObjects.insert(&sceneEntity->mListNodeOnScene);
    }

    mAABBTree.InsertObject(sceneEntity);
}

void RenderScene::DetachRenderable(Renderable* sceneEntity)
{
    debug_assert(sceneEntity);

    if (mSceneObjects.contains(&sceneEntity->mListNodeOnScene))
    {
        mSceneObjects.remove(&sceneEntity->mListNodeOnScene);
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

void RenderScene::DestroyRenderable(Renderable* sceneEntity)
{
    debug_assert(sceneEntity);

    DetachRenderable(sceneEntity);
    delete sceneEntity;
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
    while (mTransformObjects.has_elements())
    {
        cxx::intrusive_node<Renderable>* entityNode = mTransformObjects.get_head_node();
        mTransformObjects.remove(entityNode);

        // refresh aabbtree node
        Renderable* sceneEntity = entityNode->get_element();
        mAABBTree.UpdateObject(sceneEntity);
    }
}

void RenderScene::DestroySceneObjects()
{
    while (mTransformObjects.has_elements())
    {
        cxx::intrusive_node<Renderable>* entityNode = mTransformObjects.get_head_node();

        Renderable* currentEntity = entityNode->get_element();
        DestroyRenderable(currentEntity);
    }

    while (mSceneObjects.has_elements())
    {
        cxx::intrusive_node<Renderable>* entityNode = mSceneObjects.get_head_node();

        Renderable* currentEntity = entityNode->get_element();
        DestroyRenderable(currentEntity);
    }
}
