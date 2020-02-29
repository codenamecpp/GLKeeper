#include "pch.h"
#include "InputsManager.h"
#include "ImGuiManager.h"

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

    gImGuiManager.HandleInputEvent(inputEvent);
}

void InputsManager::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    mCursorPositionX = inputEvent.mCursorPositionX;
    mCursorPositionY = inputEvent.mCursorPositionY;

    gImGuiManager.HandleInputEvent(inputEvent);
}

void InputsManager::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
    gImGuiManager.HandleInputEvent(inputEvent);
}

void InputsManager::HandleInputEvent(KeyInputEvent& inputEvent)
{
    mKeyboardKeys[inputEvent.mKeycode] = inputEvent.mPressed;

    gImGuiManager.HandleInputEvent(inputEvent);
}

void InputsManager::HandleInputEvent(KeyCharEvent& inputEvent)
{
    gImGuiManager.HandleInputEvent(inputEvent);
}

void InputsManager::Clear()
{
    ::memset(mMouseButtons, 0, sizeof(mMouseButtons));
    ::memset(mKeyboardKeys, 0, sizeof(mKeyboardKeys));
}