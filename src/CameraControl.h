#pragma once

// scene camera controller base class
class SceneCameraControl
{
public:
    virtual ~SceneCameraControl()
    {
    }

    // process camera controller logic
    // @param deltaTime: Time since last frame
    virtual void HandleUpdateFrame(float deltaTime)
    {
        // do nothing
    }

    // process controller is attached to game scene
    virtual void HandleSceneAttach()
    {
        // do nothing
    }

    // process controller is detached from game scene
    virtual void HandleSceneDetach()
    {
        // do nothing
    }

    // process input event
    // @param inputEvent: Event data
    virtual void HandleInputEvent(MouseButtonInputEvent& inputEvent)
    {
        // do nothing
    }

    virtual void HandleInputEvent(MouseMovedInputEvent& inputEvent)
    {
        // do nothing
    }

    virtual void HandleInputEvent(MouseScrollInputEvent& inputEvent)
    {
        // do nothing
    }

    virtual void HandleInputEvent(KeyInputEvent& inputEvent)
    {
        // do nothing
    }

    virtual void HandleInputEvent(KeyCharEvent& inputEvent)
    {
        // do nothing
    }

public:
    SceneCamera* mSceneCamera = nullptr; // shortcut for scene.camera
};