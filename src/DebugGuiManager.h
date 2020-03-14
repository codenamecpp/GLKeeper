#pragma once

#include "InputsDefs.h"
#include "GraphicsDefs.h"
#include "DebugGuiWindow.h"

// forwards
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

    // process single frame logic
    void UpdateFrame();

    // process input event
    // @param inputEvent: Event data
    void HandleInputEvent(MouseButtonInputEvent& inputEvent);
    void HandleInputEvent(MouseMovedInputEvent& inputEvent);
    void HandleInputEvent(MouseScrollInputEvent& inputEvent);
    void HandleInputEvent(KeyInputEvent& inputEvent);
    void HandleInputEvent(KeyCharEvent& inputEvent);

    // add or remove debug window to global debug windows list
    // @param debugWindow: Window
    void AttachWindow(DebugGuiWindow* debugWindow);
    void DetachWindow(DebugGuiWindow* debugWindow);

private:

    bool AddFontFromExternalFile(ImGuiIO& imguiIO, const char* fontFile, float fontSize);
    void SetupStyle(ImGuiIO& imguiIO);

private:
    GpuBuffer* mVertsBuffer = nullptr;
    GpuBuffer* mTrisBuffer = nullptr;

    std::list<DebugGuiWindow*> mAllWindowsList;
};

extern DebugGuiManager gDebugGuiManager;
