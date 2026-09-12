#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameWorldDefs.h"
#include "GameplayDefs.h"

//////////////////////////////////////////////////////////////////////////

class GameplayCameraController: public cxx::noncopyable
{
public:
    GameplayCameraController();

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

    // get current camera info
    inline void GetCameraInfo(GameplayCameraInfo& outInfo) const
    {
        outInfo.mPosition = mCurrentPosition;
        outInfo.mHeight = mCameraHeight;
        outInfo.mRotation = mRotationAngles.y;
    }

    void FocusOnMapLocation(const Point2D& tileLocation);

    // Set camera start position / bounds
    void SetStartPosition(const glm::vec2& position);
    void SetPositionBounds(const glm::vec2& boundsMin, const glm::vec2& boundsMax);

private:
    glm::vec2 GetCameraMoveVectorFromInputs() const;

    bool GetCameraMoveDirections(EnumSet<eDirection>& cardinalDirs) const;

    float GetRotateDirectionFromInputs() const;
    float GetZoomDirectionFromInputs();

    void ClampWithinBounds(glm::vec2& position) const;
    void ClampCameraHeight(float& height) const;

    void ApplyPositionAndRotation();
    void UpdateAutoScroll(float deltaTime);

private:
    Camera* mCamera = nullptr;

    glm::vec3 mRotationAngles;
    glm::vec2 mStartPosition;
    glm::vec2 mCurrentPosition;
    glm::vec2 mMoveVelocity {};
    float mCameraHeight {};

    glm::vec2 mBoundsMin {};
    glm::vec2 mBoundsMax {};

    struct AutoScrollState
    {
        glm::vec2 mTargetPosition {};
        glm::vec2 mCurrentVelocity {};
    };
    std::optional<AutoScrollState> mAutoScroll {};

    // states
    bool mIncreasingFov;
    bool mDecreasingFov;

    bool mEnableEdgeScrolling = true;
};

//////////////////////////////////////////////////////////////////////////