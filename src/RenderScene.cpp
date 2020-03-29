#include "pch.h"
#include "RenderScene.h"
#include "SceneObject.h"
#include "ConsoleVariable.h"
#include "Console.h"
#include "CameraControl.h"
#include "TimeManager.h"
#include "DebugRenderer.h"
#include "AnimatingModel.h"
#include "SceneRenderList.h"
#include "TerrainMesh.h"
#include "WaterLavaMesh.h"
#include "TexturesManager.h"

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

    for (SceneObject* currObject: mSceneObjects)
    {
        currObject->UpdateFrame(deltaTime);
    }

    if (mCameraControl)
    {
        mCameraControl->HandleUpdateFrame(deltaTime);
    }

    BuildAABBTree();
}

void RenderScene::CollectObjectsForRendering(SceneRenderList& renderList)
{
    mCamera.ComputeMatrices();
    mAABBTree.QueryObjects(mCamera.mFrustum, [&renderList, this](SceneObject* sceneObject)
    {
        sceneObject->RegisterForRendering(renderList);
        sceneObject->mDistanceToCameraSquared = glm::length2(sceneObject->mPosition - mCamera.mPosition);
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

SceneObject* RenderScene::CreateDummyObject(const glm::vec3& position, const glm::vec3& direction, float scaling)
{
    SceneObject* sceneEntity = new SceneObject;
    sceneEntity->SetPosition(position);
    // todo : direction
    sceneEntity->SetScaling(scaling);
    return sceneEntity;
}

SceneObject* RenderScene::CreateDummyObject()
{
    SceneObject* sceneEntity = new SceneObject;
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

TerrainMesh* RenderScene::CreateTerrainMesh(const Rect2D& mapTerrainArea)
{
    TerrainMesh* sceneEntity = new TerrainMesh;
    sceneEntity->SetTerrainArea(mapTerrainArea);
    sceneEntity->InvalidateMesh();
    return sceneEntity;
}

TerrainMesh* RenderScene::CreateTerrainMesh()
{
    TerrainMesh* sceneEntity = new TerrainMesh;
    return sceneEntity;
}

WaterLavaMesh* RenderScene::CreateWaterMesh(const TilesArray& meshTiles)
{
    WaterLavaMesh* sceneEntity = new WaterLavaMesh;
    sceneEntity->SetWaterLavaTiles(meshTiles);
    sceneEntity->SetSurfaceParams(DEFAULT_WATER_TRANSLUCENCY, DEFAULT_WATER_WAVE_WIDTH, DEFAULT_WATER_WAVE_HEIGHT, DEFAULT_WATER_WAVE_FREQ, DEFAULT_WATER_LEVEL);
    sceneEntity->SetSurfaceTexture(gTexturesManager.mWaterTexture);
    return sceneEntity;
}

WaterLavaMesh* RenderScene::CreateLavaMesh(const TilesArray& meshTiles)
{
    WaterLavaMesh* sceneEntity = new WaterLavaMesh;
    sceneEntity->SetWaterLavaTiles(meshTiles);
    sceneEntity->SetSurfaceParams(DEFAULT_LAVA_TRANSLUCENCY, DEFAULT_LAVA_WAVE_WIDTH, DEFAULT_LAVA_WAVE_HEIGHT, DEFAULT_LAVA_WAVE_FREQ, DEFAULT_LAVA_LEVEL);
    sceneEntity->SetSurfaceTexture(gTexturesManager.mLavaTexture);
    return sceneEntity;
}

void RenderScene::HandleTransformChange(SceneObject* sceneEntity)
{
    debug_assert(sceneEntity);
    cxx::push_back_if_unique(mTransformObjects, sceneEntity); // queue for update
}

void RenderScene::AttachObject(SceneObject* sceneEntity)
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

void RenderScene::DetachObject(SceneObject* sceneEntity)
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

void RenderScene::DestroyObject(SceneObject* sceneEntity)
{
    debug_assert(sceneEntity);

    DetachObject(sceneEntity);
    SafeDelete(sceneEntity);
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
        SceneObject* sceneObject = mTransformObjects.back();
        mTransformObjects.pop_back();

        // refresh aabbtree node
        mAABBTree.UpdateObject(sceneObject);
    }
}

void RenderScene::DestroySceneObjects()
{
    while (!mSceneObjects.empty())
    {
        SceneObject* sceneObject = mSceneObjects.back();
        DestroyObject(sceneObject);
    }

    debug_assert(mTransformObjects.empty());
    mTransformObjects.clear();
}
