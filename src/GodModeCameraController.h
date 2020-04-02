#pragma once

#include "CameraController.h"

class GodModeCameraController: public CameraController
{
public:
    GodModeCameraController();

    // set 3rd person camera params
    // @param yawDegrees, pitchDegrees: Yaw and pitch specified in degrees
    // @param distance: Distance to focus point
    void Set3rdPersonParams(float yawDegrees, float pitchDegrees, float distance);

    // set camera focus position
    // @param position: Position
    void SetFocusPoint(const glm::vec3& position);

    // stop moving or rotating
    void StopCameraActivity();

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
    void SetupCameraProjection();

private:
    // current viewer params
    float mDegYaw;
    float mDegPitch;
    float mDistance;
    float mFOVy;

    glm::vec3 mFocusPoint;

    // current activity flags
    bool mIncreasingFov;
    bool mDecreasingFov;

    bool mIncreasingPitch;
    bool mDecreasingPitch;

    bool mZoomingIn;
    bool mZoomingOut;

    bool mMovingN;
    bool mMovingE;
    bool mMovingS;
    bool mMovingW;
    bool mMovingAltMode;
};
