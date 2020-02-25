#pragma once

#include "InputsDefs.h"
#include "GraphicsDefs.h"

// forwards
struct ImGuiIO;

// immediate mode gui manager
class ImGuiManager: public cxx::noncopyable
{
public:

    // setup ingui manager internal resources
    bool Initialize();
    void Deinit();

    // render 
    void RenderFrame();

    // process logic
    // @param deltaTime: Time passed since previous update
    void UpdateFrame();

    // process input event
    // @param inputEvent: Event data
    void HandleInputEvent(MouseButtonInputEvent& inputEvent);
    void HandleInputEvent(MouseMovedInputEvent& inputEvent);
    void HandleInputEvent(MouseScrollInputEvent& inputEvent);
    void HandleInputEvent(KeyInputEvent& inputEvent);
    void HandleInputEvent(KeyCharEvent& inputEvent);

    bool IsInitialized() const;

private:

    bool AddFontFromExternalFile(ImGuiIO& imguiIO, const char* fontFile, float fontSize);
    void SetupStyle(ImGuiIO& imguiIO);

private:
    bool mInitialized = false;

    // geometry buffers
    GpuBuffer* mVertsBuffer = nullptr;
    GpuBuffer* mTrisBuffer = nullptr;
};

extern ImGuiManager gImGuiManager;
