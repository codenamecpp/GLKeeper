#pragma once

#include "GenericGamestate.h"
#include "OrbitCameraController.h"
#include "ToolsUIMeshViewWindow.h"

// mesh browser utility gamestate
class MeshViewGamestate: public GenericGamestate
{
    friend class ToolsUIMeshViewWindow;

public:
    // enter game state
    void HandleGamestateEnter() override;

    // leave game state
    void HandleGamestateLeave() override;

    // process game state frame logic
    // @param theDeltaTime: Time since previous frame
    void HandleUpdateFrame() override;

    // process input event
    // @param inputEvent: Event data
    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;
    void HandleInputEvent(MouseMovedInputEvent& inputEvent) override;
    void HandleInputEvent(MouseScrollInputEvent& inputEvent) override;
    void HandleInputEvent(KeyInputEvent& inputEvent) override;
    void HandleInputEvent(KeyCharEvent& inputEvent) override;

    void HandleScreenResolutionChanged() override;

private:
    OrbitCameraController mOrbitCameraController;
    Entity* mModelObject = nullptr;
    ToolsUIMeshViewWindow mMeshViewWindow;
};
