#pragma once

//////////////////////////////////////////////////////////////////////////

#include "ToolsUi.h"

//////////////////////////////////////////////////////////////////////////

// immediate mode ui manager
class ToolsUiManager: public cxx::noncopyable
{
public:
    // setup ingui manager internal resources
    bool Initialize();
    void Shutdown();

    void RegisterUi(ToolsUi* ui);
    void UnregisterUi(ToolsUi* ui);

    bool IsInitialized() const;

    // render 
    void RenderFrame();

    // process single frame logic
    void UpdateFrame(float deltaTime);

    // process input event
    // @param inputEvent: Event data
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);

private:
    void SetupStyle(ImGuiIO& imguiIO);
    bool AddFontFromExternalFile(ImGuiIO& imguiIO, const char* fontFile, float fontSize);

private:
    std::vector<ToolsUi*> mUiList;
    // render data
    std::unique_ptr<GpuVertexBuffer> mVerticesBuffer;
    std::unique_ptr<GpuIndexBuffer> mIndicesBuffer;
    std::unique_ptr<GpuTexture2D> mFontTexture;
};

//////////////////////////////////////////////////////////////////////////

extern ToolsUiManager gToolsUiManager;

//////////////////////////////////////////////////////////////////////////