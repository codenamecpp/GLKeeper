#include "pch.h"
#include "InputsManager.h"
#include "DebugUiManager.h"
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

    if (gDebugUiManager.IsInitialized())
    {
        gDebugUiManager.HandleInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.HandleInputEvent(inputEvent);
    }
}

void InputsManager::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    mCursorPositionX = inputEvent.mCursorPositionX;
    mCursorPositionY = inputEvent.mCursorPositionY;

    if (gDebugUiManager.IsInitialized())
    {
        gDebugUiManager.HandleInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.HandleInputEvent(inputEvent);
    }
}

void InputsManager::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (gDebugUiManager.IsInitialized())
    {
        gDebugUiManager.HandleInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.HandleInputEvent(inputEvent);
    }
}

void InputsManager::HandleInputEvent(KeyInputEvent& inputEvent)
{
    mKeyboardKeys[inputEvent.mKeycode] = inputEvent.mPressed;

    if (gDebugUiManager.IsInitialized())
    {
        gDebugUiManager.HandleInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.HandleInputEvent(inputEvent);
    }
}

void InputsManager::HandleInputEvent(KeyCharEvent& inputEvent)
{
    if (gDebugUiManager.IsInitialized())
    {
        gDebugUiManager.HandleInputEvent(inputEvent);
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