#pragma once

// inputs manager class
class InputsManager: public cxx::noncopyable
{
public:
    // readonly
    bool mMouseButtons[eMouseButton_COUNT];
    bool mKeyboardKeys[eKeycode_COUNT];

    Point mCursorPosition;

public:
    
    // setup inputs manager internal resources, returns false on error
    bool Initialize();
    void Deinit();

    // process input event
    // @param inputEvent: Event data
    void ProcessInputEvent(MouseButtonInputEvent& inputEvent);
    void ProcessInputEvent(MouseMovedInputEvent& inputEvent);
    void ProcessInputEvent(MouseScrollInputEvent& inputEvent);
    void ProcessInputEvent(KeyInputEvent& inputEvent);
    void ProcessInputEvent(KeyCharEvent& inputEvent);

    // reset all keys and mouse buttons state
    void Clear();

    // set current mouse position
    // @param positionx: position x
    // @param positiony: position y
    void SetMousePosition(const Point& cursorPosition);

    // set current mouse button state
    // @param button: mouse button index
    // @param buttonState: button pressed/released
    void SetMouseButtonState(eMouseButton button, bool state)
    {
        debug_assert(button < eMouseButton_COUNT && button > eMouseButton_null);
        mMouseButtons[button] = state;
    }
    // set current key state
    // @param keycode: keycode
    // @param keystate: key pressed/released
    void SetKeyState(eKeycode keycode, bool state)
    {
        debug_assert(keycode < eKeycode_COUNT && keycode > eKeycode_null);
        mKeyboardKeys[keycode] = state;
    }
    // get current key state
    // @param keycode: keycode
    bool GetKeyState(eKeycode keycode) const 
    {
        debug_assert(keycode < eKeycode_COUNT && keycode > eKeycode_null);
        return mKeyboardKeys[keycode];
    }
    bool GetMouseButtonL() const { return mMouseButtons[eMouseButton_Left]; }
    bool GetMouseButtonR() const { return mMouseButtons[eMouseButton_Right]; }
    bool GetMouseButtonM() const { return mMouseButtons[eMouseButton_Middle]; }
    // Get current mouse button state
    // @param button: mouse button index
    bool GetMouseButton(eMouseButton button) const 
    {
        debug_assert(button < eMouseButton_COUNT && button > eMouseButton_null);
        return mMouseButtons[button];
    }
};

extern InputsManager gInputsManager;