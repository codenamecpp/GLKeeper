#pragma once

#include "CommonTypes.h"

// Key codes

enum
{
    KEYCODE_ESCAPE      = GLFW_KEY_ESCAPE,
    KEYCODE_SPACE       = GLFW_KEY_SPACE,
    KEYCODE_PAGE_UP     = GLFW_KEY_PAGE_UP,
    KEYCODE_PAGE_DOWN   = GLFW_KEY_PAGE_DOWN,
    KEYCODE_HOME        = GLFW_KEY_HOME,
    KEYCODE_RIGHT_CTRL  = GLFW_KEY_RIGHT_CONTROL,
    KEYCODE_LEFT_CTRL   = GLFW_KEY_LEFT_CONTROL,
    KEYCODE_BACKSPACE   = GLFW_KEY_BACKSPACE,
    KEYCODE_ENTER       = GLFW_KEY_ENTER,
    KEYCODE_TAB         = GLFW_KEY_TAB,
    KEYCODE_TILDE       = GLFW_KEY_GRAVE_ACCENT,
    KEYCODE_END         = GLFW_KEY_END,
    KEYCODE_INSERT      = GLFW_KEY_INSERT,
    KEYCODE_DELETE      = GLFW_KEY_DELETE,

    KEYCODE_F1          = GLFW_KEY_F1,
    KEYCODE_F2          = GLFW_KEY_F2,
    KEYCODE_F3          = GLFW_KEY_F3,
    KEYCODE_F4          = GLFW_KEY_F4,
    KEYCODE_F5          = GLFW_KEY_F5,
    KEYCODE_F6          = GLFW_KEY_F6,
    KEYCODE_F7          = GLFW_KEY_F7,
    KEYCODE_F8          = GLFW_KEY_F8,
    KEYCODE_F9          = GLFW_KEY_F9,
    KEYCODE_F10         = GLFW_KEY_F10,
    KEYCODE_F11         = GLFW_KEY_F11,
    KEYCODE_F12         = GLFW_KEY_F12,

    // Letters
    KEYCODE_A           = GLFW_KEY_A,
    KEYCODE_W           = GLFW_KEY_W,
    KEYCODE_D           = GLFW_KEY_D,
    KEYCODE_S           = GLFW_KEY_S,

    KEYCODE_R           = GLFW_KEY_R,

    // digits
    KEYCODE_0           = GLFW_KEY_0,
    KEYCODE_1           = GLFW_KEY_1,
    KEYCODE_2           = GLFW_KEY_2,
    KEYCODE_3           = GLFW_KEY_3,
    KEYCODE_4           = GLFW_KEY_4,
    KEYCODE_5           = GLFW_KEY_5,
    KEYCODE_6           = GLFW_KEY_6,
    KEYCODE_7           = GLFW_KEY_7,
    KEYCODE_8           = GLFW_KEY_8,
    KEYCODE_9           = GLFW_KEY_9,
        
    // Arrow keys
    KEYCODE_LEFT        = GLFW_KEY_LEFT,
    KEYCODE_RIGHT       = GLFW_KEY_RIGHT,
    KEYCODE_UP          = GLFW_KEY_UP,
    KEYCODE_DOWN        = GLFW_KEY_DOWN,

    // mouse buttons
    MBUTTON_LEFT        = GLFW_MOUSE_BUTTON_LEFT,
    MBUTTON_RIGHT       = GLFW_MOUSE_BUTTON_RIGHT,
    MBUTTON_MIDDLE      = GLFW_MOUSE_BUTTON_MIDDLE
};

// Key mods

enum
{
    KEYMOD_SHIFT    = GLFW_MOD_SHIFT,
    KEYMOD_CTRL     = GLFW_MOD_CONTROL,
    KEYMOD_ALT      = GLFW_MOD_ALT
};

//////////////////////////////////////////////////////////////////////////

// Input events

struct BaseInputEvent
{
public:
    BaseInputEvent() = default;
    // Set consumed flag
    void SetConsumed(bool isConsumed = true)
    {
        mConsumed = isConsumed;
    }
public:
    bool mConsumed {}; // Whether event is already consumed
};

struct KeyInputEvent : public BaseInputEvent
{
public:
    KeyInputEvent() = default;
    KeyInputEvent(int argKeycode, int argScancode, int argMods, bool argIsPressed)
        : mKeycode(argKeycode)
        , mScancode(argScancode)
        , mMods(argMods)
        , mPressed(argIsPressed)
    {}
    inline bool IsKeyPressed(int keycode) const { return mPressed && (keycode == mKeycode); }
    inline bool IsKeyReleased(int keycode) const { return !mPressed && (keycode == mKeycode); }
    inline bool HasModifiers(int bits) const { return (mMods & bits) == bits; }
public:
    int mKeycode;
    int mScancode;
    int mMods;
    bool mPressed; 
};

struct KeyCharEvent: public BaseInputEvent
{
public:
    KeyCharEvent() = default;
    KeyCharEvent(unsigned int unicodeChar): mUnicodeChar(unicodeChar)
    {}
public:
    unsigned int mUnicodeChar;
};

struct MouseButtonInputEvent : public BaseInputEvent
{
public:
    MouseButtonInputEvent() = default;
    MouseButtonInputEvent(int argButton, int argMods, bool argIsPressed)
        : mButton(argButton)
        , mMods(argMods)
        , mPressed(argIsPressed)
    {}
    inline bool IsButtonPressed(int button) const { return mPressed && (button == mButton); }
    inline bool IsButtonReleased(int button) const { return !mPressed && (button == mButton); }
    inline bool HasModifiers(int bits) const { return (mMods & bits) == bits; }
public:
    Point2D mMousePosition;
    int mButton;
    int mMods;
    bool mPressed;
};

struct MouseMovedInputEvent : public BaseInputEvent
{
public:
    MouseMovedInputEvent() = default;
    MouseMovedInputEvent(const Point2D& mousePosition)
        : mMousePosition(mousePosition)
        , mDelta(0, 0)
    {}
public:
    Point2D mMousePosition;
    Point2D mDelta;
};

struct MouseScrollInputEvent : public BaseInputEvent
{
public:
    MouseScrollInputEvent() = default;
    MouseScrollInputEvent(const Point2D& scroll)
        : mMousePosition(0, 0)
        , mScroll(scroll)
    {}
public:
    Point2D mMousePosition;
    Point2D mScroll;
};

//////////////////////////////////////////////////////////////////////////
// Input State Holder
//////////////////////////////////////////////////////////////////////////

class InputsState
{
public:
    InputsState();

    // Reset all keys and mouse buttons state
    void Cleanup();

    // Set current mouse position
    inline void SetMousePosition(const Point2D& mousePosition)
    {
        mMousePosition = mousePosition;
    }

    inline const Point2D& GetMousePosition() const { return mMousePosition; }

    // Set current mouse button state
    inline void SetMouseButtonState(int button, bool state)
    {
        cxx_assert((button >= 0) && (button <= GLFW_MOUSE_BUTTON_LAST));
        mMouseButtons[button] = state;
    }

    // Set current key state
    inline void SetKeyState(int keycode, bool state)
    {
        cxx_assert((keycode >= 0) && (keycode <= GLFW_KEY_LAST));
        mKeyboardKeys[keycode] = state;
    }

    // Get current key state
    inline bool GetKeyState(int keycode) const 
    {
        cxx_assert((keycode >= 0) && (keycode <= GLFW_KEY_LAST));
        return mKeyboardKeys[keycode];
    }

    inline bool GetMouseLeftButton() const { return mMouseButtons[GLFW_MOUSE_BUTTON_LEFT]; }
    inline bool GetMouseRightButton() const { return mMouseButtons[GLFW_MOUSE_BUTTON_RIGHT]; }
    inline bool GetMouseMiddleButton() const { return mMouseButtons[GLFW_MOUSE_BUTTON_MIDDLE]; }

    // Get current mouse button state
    // @param button: mouse button index
    inline bool GetMouseButton(int button) const 
    {
        cxx_assert((button >= 0) && (button <= GLFW_MOUSE_BUTTON_LAST));
        return mMouseButtons[button];
    }

private:
    bool mMouseButtons[GLFW_MOUSE_BUTTON_LAST + 1];
    bool mKeyboardKeys[GLFW_KEY_LAST + 1];
    Point2D mMousePosition {};
};

extern InputsState gInputs;