#include "pch.h"
#include "MeshViewGamestate.h"
#include "GameMain.h"
#include "RenderScene.h"
#include "System.h"
#include "GameObject.h"
#include "ModelAssetsManager.h"
#include "AnimatingModel.h"
#include "TimeManager.h"
#include "DebugUiManager.h"
#include "GameObjectsManager.h"

//////////////////////////////////////////////////////////////////////////

#define MESH_VIEW_CAMERA_YAW_DEG    90.0f
#define MESH_VIEW_CAMERA_PITCH_DEG  75.0f
#define MESH_VIEW_CAMERA_DISTANCE   3.0f
#define MESH_VIEW_CAMERA_NEAR       0.01f
#define MESH_VIEW_CAMERA_FAR        100.0f
#define MESH_VIEW_CAMERA_FOVY       60.0f

//////////////////////////////////////////////////////////////////////////

void MeshViewGamestate::HandleGamestateEnter()
{
    mOrbitCameraControl.SetParams(MESH_VIEW_CAMERA_YAW_DEG, MESH_VIEW_CAMERA_PITCH_DEG, MESH_VIEW_CAMERA_DISTANCE);

    // setup scene camera
    gRenderScene.mCamera.SetPerspective(gSystem.mSettings.mScreenAspectRatio, MESH_VIEW_CAMERA_FOVY, MESH_VIEW_CAMERA_NEAR, MESH_VIEW_CAMERA_FAR);

    gRenderScene.SetCameraControl(&mOrbitCameraControl);
    mOrbitCameraControl.ResetOrientation();

    ModelAsset* modelAsset = gModelsManager.LoadModelAsset("vampire-pray");

    mModelObject = gGameObjectsManager.CreateGameObject();
    debug_assert(mModelObject);

    AnimatingModel* animComponent = new AnimatingModel(mModelObject);
    mModelObject->AddComponent(animComponent);

    animComponent->SetModelAsset(modelAsset);
    animComponent->StartAnimation(24.0f, true);
    gRenderScene.AttachObject(mModelObject);

    gDebugUiManager.AttachWindow(&mMeshViewWindow);
    mMeshViewWindow.SetWindowShown(true);
    mMeshViewWindow.Setup(this);
}

void MeshViewGamestate::HandleGamestateLeave()
{
    gRenderScene.SetCameraControl(nullptr);
    if (mModelObject)
    {
        gRenderScene.DetachObject(mModelObject);
        gGameObjectsManager.DestroyGameObject(mModelObject);
        mModelObject = nullptr;
    }

    gDebugUiManager.DetachWindow(&mMeshViewWindow);
}

void MeshViewGamestate::HandleUpdateFrame()
{
}

void MeshViewGamestate::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void MeshViewGamestate::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void MeshViewGamestate::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void MeshViewGamestate::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void MeshViewGamestate::HandleInputEvent(KeyCharEvent& inputEvent)
{
}