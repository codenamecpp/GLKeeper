#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CameraPath.h"

//////////////////////////////////////////////////////////////////////////

class Camera;
class FrontendController;

//////////////////////////////////////////////////////////////////////////

// Front End main menu camera controller
class FrontendCameraController: public cxx::noncopyable
{
public:
    
    //////////////////////////////////////////////////////////////////////////

    enum eWorkMode { eWorkMode_Default, eWorkMode_InTransition };
    enum eLocation 
    {
        eLocation_Entrance,
        eLocation_Table,
        eLocation_1stRight,
        eLocation_1stLeft,
        eLocation_2ndRight,
        eLocation_2ndLeft,
        eLocation_CreditView,
        eLocation_COUNT
    };

    //////////////////////////////////////////////////////////////////////////

public:
    FrontendCameraController(FrontendController& froented);

    // Set controllable camera and setup it to initial state
    void CaptureCamera(Camera* camera);
    void ReleaseCamera();

    // Update controller logic
    void UpdateFrame(float deltaTime);

    // Process input event
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);

    // Reset to defaults
    void ResetCamera();

    // Stop moving or rotating
    void StopCamera();

    // Set camera start position
    void SetStartPosition(const glm::vec3& position);

    // request transitions
    void StartTransitionToLocation(eLocation newLocation);

private:
    void InitTransitionsMatrix();

private:
    FrontendController& mFrontend;
    Camera* mCamera {};
    glm::vec3 mStartPosition;
    eWorkMode mCurrentMode = eWorkMode_Default;
    eLocation mCurrentLocation = eLocation_Entrance;
    CameraPathId mTransitionsMatrix[eLocation_COUNT][eLocation_COUNT]; // src, dst
};

//////////////////////////////////////////////////////////////////////////