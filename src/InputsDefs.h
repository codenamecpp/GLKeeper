#pragma once

// mouse buttons
enum eMouseButton
{
    eMouseButton_null, // invalid mouse button

    eMouseButton_Left,
    eMouseButton_Right,
    eMouseButton_Middle,
    eMouseButton_COUNT
};

decl_enum_strings(eMouseButton);

// keys
enum eKeycode
{
    eKeycode_null, // invalid keycode

    eKeycode_ESCAPE,
    eKeycode_SPACE,
    eKeycode_PAGE_UP,
    eKeycode_PAGE_DOWN,
    eKeycode_HOME,
    eKeycode_END,
    eKeycode_INSERT,
    eKeycode_DELETE,
    eKeycode_RIGHT_CTRL,
    eKeycode_LEFT_CTRL,
    eKeycode_BACKSPACE,
    eKeycode_ENTER,
    eKeycode_TAB,
    eKeycode_TILDE,

    eKeycode_F1,
    eKeycode_F2,
    eKeycode_F3,
    eKeycode_F4,
    eKeycode_F5,
    eKeycode_F6,
    eKeycode_F7,
    eKeycode_F8,
    eKeycode_F9,
    eKeycode_F10,
    eKeycode_F11,
    eKeycode_F12,

    eKeycode_A,
    eKeycode_C,
    eKeycode_F,
    eKeycode_V,
    eKeycode_X,
    eKeycode_W,
    eKeycode_D,
    eKeycode_S,
    eKeycode_Y,
    eKeycode_Z,
    eKeycode_R,

    eKeycode_0,
    eKeycode_1,
    eKeycode_2,
    eKeycode_3,
    eKeycode_4,
    eKeycode_5,
    eKeycode_6,
    eKeycode_7,
    eKeycode_8,
    eKeycode_9,

    eKeycode_LEFT,
    eKeycode_RIGHT,
    eKeycode_UP,
    eKeycode_DOWN,

    eKeycode_COUNT
};

decl_enum_strings(eKeycode);

// input events
struct BaseInputEvent
{
public:
    BaseInputEvent() = default;
    inline void SetConsumed(bool isConsumed = true)
    {
        mConsumed = isConsumed;
    }
public:
    bool mConsumed = false; // Whether event is already consumed
};

struct KeyInputEvent: public BaseInputEvent
{
public:
    KeyInputEvent() = default;
    KeyInputEvent(eKeycode keycode, int scancode, int mods, bool ispressed)
        : mKeycode(keycode)
        , mScancode(scancode)
        , mMods(mods)
        , mPressed(ispressed)
    {
    }
    // test whether modifier keys are pressed
    inline bool HasModifiers(int bits) const
    {
        return (mMods & bits) == bits;
    }
public:
    eKeycode mKeycode;
    int mScancode;
    int mMods;
    bool mPressed; 
};

struct KeyCharEvent: public BaseInputEvent
{
public:
    KeyCharEvent() = default;
    KeyCharEvent(unsigned int unicodeChar): mUnicodeChar(unicodeChar)
    {
    }
public:
    unsigned int mUnicodeChar;
};

struct MouseButtonInputEvent: public BaseInputEvent
{
public:
    MouseButtonInputEvent() = default;
    MouseButtonInputEvent(eMouseButton mbutton, int mods, bool ispressed)
        : mButton(mbutton)
        , mMods(mods)
        , mPressed(ispressed)
    {
    }
    // test whether modifier keys are pressed
    inline bool HasModifiers(int bits) const
    {
        return (mMods & bits) == bits;
    }
public:
    eMouseButton mButton;
    int mMods;
    bool mPressed;
};

struct MouseMovedInputEvent: public BaseInputEvent
{
public:
    MouseMovedInputEvent() = default;
    MouseMovedInputEvent(int posx, int posy)
        : mCursorPositionX(posx)
        , mCursorPositionY(posy)
        , mDeltaX()
        , mDeltaY()
    {
    }
public:
    int mCursorPositionX;
    int mCursorPositionY;
    int mDeltaX;
    int mDeltaY;
};

struct MouseScrollInputEvent: public BaseInputEvent
{
public:
    MouseScrollInputEvent() = default;
    MouseScrollInputEvent(int scrollx, int scrolly)
        : mScrollX(scrollx)
        , mScrollY(scrolly)
    {
    }
public:
    int mScrollX;
    int mScrollY;
};
