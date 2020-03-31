#include "pch.h"
#include "MeshViewGamestate.h"
#include "GameMain.h"
#include "RenderScene.h"
#include "System.h"
#include "SceneObject.h"
#include "ModelAssetsManager.h"
#include "AnimatingModelComponent.h"
#include "TimeManager.h"
#include "DebugUiManager.h"

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

    mModelObject = gRenderScene.CreateAnimatingModel(modelAsset, glm::vec3(0.0f), glm::vec3(0.0f));
    if (mModelObject)
    {
        gRenderScene.AttachObject(mModelObject);

        AnimatingModelComponent* component = mModelObject->GetAnimatingModelComponent();
        debug_assert(component);

        component->StartAnimation(24.0f, true);
    }

    gDebugUiManager.AttachWindow(&mMeshViewWindow);
    mMeshViewWindow.SetWindowShown(true);
    mMeshViewWindow.Setup(this);
}

void MeshViewGamestate::HandleGamestateLeave()
{
    gRenderScene.SetCameraControl(nullptr);
    if (mModelObject)
    {
        gRenderScene.DestroyObject(mModelObject);
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