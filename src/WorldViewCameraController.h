#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameWorldDefs.h"

//////////////////////////////////////////////////////////////////////////

class WorldViewCameraController: public cxx::noncopyable
{
public:
    WorldViewCameraController();

    // Set controllable camera and setup it to initial state
    void CaptureCamera(Camera* camera);
    void ReleaseCamera();

    // Update controller logic
    // @param deltaTime: Time since last frame in seconds
    void UpdateFrame(float deltaTime);

    // Process input event
    // @param inputEvent
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);

    // Reset to defaults
    void ResetCamera();

    // Stop moving or rotating
    void StopCamera();

    // Set camera start position
    // @param position: Position
    void SetStartPosition(const glm::vec3& position);

private:
    glm::vec3 GetMoveDirectionFromInputs() const;

    float GetRotateDirectionFromInputs() const;
    float GetZoomDirectionFromInputs();

private:
    Camera* mCamera = nullptr;

    glm::vec3 mRotationAngles;
    glm::vec3 mStartPosition;

    glm::vec3 mMoveVelocity {};

    // states
    bool mIncreasingFov;
    bool mDecreasingFov;
};