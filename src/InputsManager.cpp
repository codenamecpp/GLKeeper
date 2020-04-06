#include "pch.h"
#include "InputsManager.h"
#include "DebugUIManager.h"
#include "GameMain.h"

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

    if (gDebugUIManager.IsInitialized())
    {
        gDebugUIManager.HandleInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.HandleInputEvent(inputEvent);
    }
}

void InputsManager::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    mCursorPosition.x = inputEvent.mCursorPositionX;
    mCursorPosition.y = inputEvent.mCursorPositionY;

    if (gDebugUIManager.IsInitialized())
    {
        gDebugUIManager.HandleInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.HandleInputEvent(inputEvent);
    }
}

void InputsManager::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (gDebugUIManager.IsInitialized())
    {
        gDebugUIManager.HandleInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.HandleInputEvent(inputEvent);
    }
}

void InputsManager::HandleInputEvent(KeyInputEvent& inputEvent)
{
    mKeyboardKeys[inputEvent.mKeycode] = inputEvent.mPressed;

    if (gDebugUIManager.IsInitialized())
    {
        gDebugUIManager.HandleInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.HandleInputEvent(inputEvent);
    }
}

void InputsManager::HandleInputEvent(KeyCharEvent& inputEvent)
{
    if (gDebugUIManager.IsInitialized())
    {
        gDebugUIManager.HandleInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.HandleInputEvent(inputEvent);
    }
}

void InputsManager::Clear()
{
    ::memset(mMouseButtons, 0, sizeof(mMouseButtons));
    ::memset(mKeyboardKeys, 0, sizeof(mKeyboardKeys));
}

void InputsManager::SetMousePosition(const Point& cursorPosition)
{
    mCursorPosition = cursorPosition;
}
