#include "pch.h"
#include "MeshViewGamestate.h"
#include "GameMain.h"
#include "RenderScene.h"
#include "System.h"
#include "Renderable.h"
#include "ModelAssetsManager.h"
#include "AnimatingModel.h"
#include "TimeManager.h"

#define MESH_VIEW_CAMERA_YAW_DEG    90.0f
#define MESH_VIEW_CAMERA_PITCH_DEG  75.0f
#define MESH_VIEW_CAMERA_DISTANCE   3.0f
#define MESH_VIEW_CAMERA_NEAR       0.01f
#define MESH_VIEW_CAMERA_FAR        100.0f
#define MESH_VIEW_CAMERA_FOVY       60.0f

AnimatingModel* sceneObject;

void MeshViewGamestate::HandleGamestateEnter()
{
    mOrbitCameraControl.SetParams(MESH_VIEW_CAMERA_YAW_DEG, MESH_VIEW_CAMERA_PITCH_DEG, MESH_VIEW_CAMERA_DISTANCE);

    // setup scene camera
    gRenderScene.mCamera.SetPerspective(gSystem.mSettings.mScreenAspectRatio, MESH_VIEW_CAMERA_FOVY, MESH_VIEW_CAMERA_NEAR, MESH_VIEW_CAMERA_FAR);

    gRenderScene.SetCameraControl(&mOrbitCameraControl);
    mOrbitCameraControl.ResetOrientation();

    ModelAsset* modelAsset = gModelsManager.LoadModelAsset("vampire-pray.kmf");

    sceneObject = gRenderScene.CreateAnimatingModel(modelAsset, glm::vec3(0.0f), glm::vec3(0.0f));
    gRenderScene.AttachRenderable(sceneObject);
    sceneObject->StartAnimation(20.0f, true);
}

void MeshViewGamestate::HandleGamestateLeave()
{
}

void MeshViewGamestate::HandleUpdateFrame()
{
    float dt = (float) gTimeManager.GetRealtimeFrameDelta();

    sceneObject->AdvanceAnimation(dt);
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