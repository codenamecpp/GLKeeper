#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Font.h"

//////////////////////////////////////////////////////////////////////////

class UiManager
{
public:
    bool Initialize();
    void Shutdown();

    // Process gui logic
    void UpdateFrame(float deltaTime);

    // Process input events
    // @param inputEvent: Input event data
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);

    // font helpers
    Font* GetDefaultFont(eDefaultFont id) const;

public:
    // notifications
    void ScreenSizeChanged();

private:
    void PreloadBaseFonts();

private:
    Font* mFpsFont = nullptr;
    Font* mConsoleFont = nullptr;
};

//////////////////////////////////////////////////////////////////////////

extern UiManager gUiManager;

//////////////////////////////////////////////////////////////////////////