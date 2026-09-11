#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Font.h"

//////////////////////////////////////////////////////////////////////////

class UiManager
{
public:

    bool Initialize();
    void Shutdown();

    void RenderFrame(UiRenderContext& renderContext);
    void RenderFrameOverlay(UiRenderContext& renderContext);
    void UpdateFrame(float deltaTime);

    // Process input events
    // @param inputEvent: Input event data
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);

    // whether some ui element under cursor right now
    bool IsCursorOverUi() const; 

    inline const Rect2D& GetScreenRect() const { return mScreenRect; }

    // font helpers
    Font* GetDefaultFont(eDefaultFont id) const;

public:
    // notifications
    void ScreenSizeChanged(const Point2D& screenSize);

private:
    void PreloadBaseFonts();

private:
    Rect2D mScreenRect {};
    Font* mFpsFont = nullptr;
    Font* mConsoleFont = nullptr;
};

//////////////////////////////////////////////////////////////////////////

extern UiManager gUiManager;

//////////////////////////////////////////////////////////////////////////