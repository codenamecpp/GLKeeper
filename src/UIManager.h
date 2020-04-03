#pragma once

class UIManager: public cxx::noncopyable
{
public:

    // setup ui manager internal resources
    bool Initialize();
    void Deinit();

    // render
    void RenderFrame(UIRenderer& renderContext);

    // process single frame logic
    void UpdateFrame();

    // process input event
    // @param inputEvent: Event data
    void HandleInputEvent(MouseButtonInputEvent& inputEvent);
    void HandleInputEvent(MouseMovedInputEvent& inputEvent);
    void HandleInputEvent(MouseScrollInputEvent& inputEvent);
    void HandleInputEvent(KeyInputEvent& inputEvent);
    void HandleInputEvent(KeyCharEvent& inputEvent);
};

extern UIManager gUIManager;