#pragma once

#include "DebugUIWindow.h"

// forwards
struct ImGuiIO;

// immediate mode gui manager
class DebugUIManager: public cxx::noncopyable
{
public:

    // setup ingui manager internal resources
    bool Initialize();
    void Deinit();

    bool IsInitialized() const;

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
    void AttachWindow(DebugUIWindow* debugWindow);
    void DetachWindow(DebugUIWindow* debugWindow);

private:

    bool AddFontFromExternalFile(ImGuiIO& imguiIO, const char* fontFile, float fontSize);
    void SetupStyle(ImGuiIO& imguiIO);

private:
    // render data
    GpuBuffer* mVerticesBuffer = nullptr;
    GpuBuffer* mIndicesBuffer = nullptr;

    std::list<DebugUIWindow*> mAllWindowsList;
};

extern DebugUIManager gDebugUIManager;
