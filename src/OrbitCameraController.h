#pragma once

#include "CameraController.h"

class OrbitCameraController: public CameraController
{
public:
    OrbitCameraController();

    // set camera control default params
    void SetParams(float defaultYawDegrees, float defaultPitchDegrees, float defaultDistance);

    // set camera orientation to default params
    void ResetOrientation();

    // process frame logic
    void HandleUpdateFrame(float deltaTime) override;

    // process controller is attached to game scene
    void HandleSceneAttach() override;

    // process controller is detached from game scene
    void HandleSceneDetach() override;

    // process input event
    // @param inputEvent: Event data
    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;
    void HandleInputEvent(MouseMovedInputEvent& inputEvent) override;
    void HandleInputEvent(MouseScrollInputEvent& inputEvent) override;
    void HandleInputEvent(KeyInputEvent& inputEvent) override;
    void HandleInputEvent(KeyCharEvent& inputEvent) override;

private:
    void SetupCameraView();

private:
    Point2D mPrevMousePosition;

    glm::vec3 mFocusPoint;

    float mDegYaw;
    float mDegPitch;
    float mDistance;

    float mDefaultYaw;
    float mDefaultPitch;
    float mDefaultDistance;
};