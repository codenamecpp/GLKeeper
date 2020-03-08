#pragma once

#include "GameDefs.h"
#include "InputsDefs.h"
#include "RenderDefs.h"

// forwards
class GenericGamestate;

// game core
class GameMain: public cxx::noncopyable
{
public:
    // readonly
    GenericGamestate* mCurrentGamestate = nullptr;

public:

    // setup internal game resources and gamestates
    bool Initialize();
    void Deinit();

    // process single frame logic
    void UpdateFrame();

    // process debug draw
    void DebugRenderFrame(DebugRenderer& renderer);

    // process input event
    // @param inputEvent: Event data
    void HandleInputEvent(MouseButtonInputEvent& inputEvent);
    void HandleInputEvent(MouseMovedInputEvent& inputEvent);
    void HandleInputEvent(MouseScrollInputEvent& inputEvent);
    void HandleInputEvent(KeyInputEvent& inputEvent);
    void HandleInputEvent(KeyCharEvent& inputEvent);

private:

};

extern GameMain gGameMain;