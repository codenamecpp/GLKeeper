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

    // process logic
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
    void RegisterWindow(DebugGuiWindow* debugWindow);
    void UnregisterWindow(DebugGuiWindow* debugWindow);

private:

    bool AddFontFromExternalFile(ImGuiIO& imguiIO, const char* fontFile, float fontSize);
    void SetupStyle(ImGuiIO& imguiIO);

private:

    std::list<DebugGuiWindow*> mAllWindowsList;

    GpuBuffer* mVertsBuffer = nullptr;
    GpuBuffer* mTrisBuffer = nullptr;
};

extern DebugGuiManager gDebugGuiManager;
