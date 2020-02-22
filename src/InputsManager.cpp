#include "pch.h"
#include "InputsManager.h"

InputsManager gInputsManager;

bool InputsManager::Initialize()
{
    Clear();

    return true;
}

void InputsManager::Deinit()
{
}

void InputsManager::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    mMouseButtons[inputEvent.mButton] = inputEvent.mPressed;
}

void InputsManager::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    mCursorPositionX = inputEvent.mCursorPositionX;
    mCursorPositionY = inputEvent.mCursorPositionY;
}

void InputsManager::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void InputsManager::HandleInputEvent(KeyInputEvent& inputEvent)
{
    mKeyboardKeys[inputEvent.mKeycode] = inputEvent.mPressed;
}

void InputsManager::HandleInputEvent(KeyCharEvent& inputEvent)
{
}

void InputsManager::Clear()
{
    ::memset(mMouseButtons, 0, sizeof(mMouseButtons));
    ::memset(mKeyboardKeys, 0, sizeof(mKeyboardKeys));
}