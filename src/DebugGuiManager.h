#pragma once

#include "InputsDefs.h"
#include "GraphicsDefs.h"

// forwards
class DebugGuiWindow;

struct ImGuiIO;

// immediate mode gui manager
class DebugGuiManager: public cxx::noncopyable
{
public:

    // setup ingui manager internal resources
    bool Initialize();
    void Deinit();

    // render 
    void RenderFrame();

    // process logic
    void UpdateFrame();

    // process input event
    // @param inputEvent: Event data
    void HandleInputEvent(MouseButtonInputEvent& inputEvent);
    void HandleInputEvent(MouseMovedInputEvent& inputEvent);
    void HandleInputEvent(MouseScrollInputEvent& inputEvent);
    void HandleInputEvent(KeyInputEvent& inputEvent);
    void HandleInputEvent(KeyCharEvent& inputEvent);

private:

    bool AddFontFromExternalFile(ImGuiIO& imguiIO, const char* fontFile, float fontSize);
    void SetupStyle(ImGuiIO& imguiIO);

private:
    // geometry buffers
    GpuBuffer* mVertsBuffer = nullptr;
    GpuBuffer* mTrisBuffer = nullptr;
};

extern DebugGuiManager gDebugGuiManager;
