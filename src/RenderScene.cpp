#include "pch.h"
#include "RenderScene.h"
#include "SceneObject.h"
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

void RenderScene::CollectObjectsForRendering()
{
    mCamera.ComputeMatrices();
    mAABBTree.QueryObjects(mCamera.mFrustum, [this](SceneObject* sceneObject)
    {
        gRenderManager.RegisterSceneObjectForRendering(sceneObject);

        SceneObjectTransform* transformComponent = sceneObject->GetTransformComponent();
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

SceneObject* RenderScene::CreateDummyObject(const glm::vec3& position, const glm::vec3& direction, float scaling)
{
    SceneObject* sceneObject = new SceneObject;

    SceneObjectTransform* transformComponent = new SceneObjectTransform(sceneObject);
    sceneObject->AddComponent(transformComponent);

    transformComponent->SetPosition(position);
    transformComponent->SetScaling(scaling);
    // todo direction

    return sceneObject;
}

SceneObject* RenderScene::CreateDummyObject()
{
    SceneObject* sceneEntity = new SceneObject;
    return sceneEntity;
}

SceneObject* RenderScene::CreateAnimatingModel(ModelAsset* modelAsset, const glm::vec3& position, const glm::vec3& direction)
{
    SceneObject* sceneObject = CreateAnimatingModel();
    if (sceneObject)
    {
        sceneObject->GetTransformComponent()->SetPosition(position);
        // todo : direction

        sceneObject->GetAnimatingModelComponent()->SetModelAsset(modelAsset);
    }
    return sceneObject;
}

SceneObject* RenderScene::CreateAnimatingModel()
{
    SceneObject* sceneObject = new SceneObject;

    SceneObjectTransform* transformComponent = new SceneObjectTransform(sceneObject);
    sceneObject->AddComponent(transformComponent);

    AnimatingModelComponent* modelComponent = new AnimatingModelComponent(sceneObject);
    sceneObject->AddComponent(modelComponent);

    return sceneObject;
}

SceneObject* RenderScene::CreateTerrainMesh(const Rect2D& mapTerrainArea)
{
    SceneObject* sceneObject = CreateTerrainMesh();

    sceneObject->GetTerrainMeshComponent()->SetTerrainArea(mapTerrainArea);
    sceneObject->GetTerrainMeshComponent()->InvalidateMesh();

    return sceneObject;
}

SceneObject* RenderScene::CreateTerrainMesh()
{
    SceneObject* sceneObject = new SceneObject;

    SceneObjectTransform* transformComponent = new SceneObjectTransform(sceneObject);
    sceneObject->AddComponent(transformComponent);

    TerrainMeshComponent* meshComponent = new TerrainMeshComponent(sceneObject);
    sceneObject->AddComponent(meshComponent);

    return sceneObject;
}

SceneObject* RenderScene::CreateWaterMesh(const TilesArray& meshTiles)
{
    SceneObject* sceneObject = new SceneObject;

    SceneObjectTransform* transformComponent = new SceneObjectTransform(sceneObject);
    sceneObject->AddComponent(transformComponent);

    WaterLavaMeshComponent* meshComponent = new WaterLavaMeshComponent(sceneObject);
    sceneObject->AddComponent(meshComponent);

    meshComponent->SetWaterLavaTiles(meshTiles);
    meshComponent->SetSurfaceParams(DEFAULT_WATER_TRANSLUCENCY, DEFAULT_WATER_WAVE_WIDTH, DEFAULT_WATER_WAVE_HEIGHT, DEFAULT_WATER_WAVE_FREQ, DEFAULT_WATER_LEVEL);
    meshComponent->SetSurfaceTexture(gTexturesManager.mWaterTexture);

    return sceneObject;
}

SceneObject* RenderScene::CreateLavaMesh(const TilesArray& meshTiles)
{
    SceneObject* sceneObject = new SceneObject;

    SceneObjectTransform* transformComponent = new SceneObjectTransform(sceneObject);
    sceneObject->AddComponent(transformComponent);

    WaterLavaMeshComponent* meshComponent = new WaterLavaMeshComponent(sceneObject);
    sceneObject->AddComponent(meshComponent);

    meshComponent->SetWaterLavaTiles(meshTiles);
    meshComponent->SetSurfaceParams(DEFAULT_LAVA_TRANSLUCENCY, DEFAULT_LAVA_WAVE_WIDTH, DEFAULT_LAVA_WAVE_HEIGHT, DEFAULT_LAVA_WAVE_FREQ, DEFAULT_LAVA_LEVEL);
    meshComponent->SetSurfaceTexture(gTexturesManager.mLavaTexture);

    return sceneObject;
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
