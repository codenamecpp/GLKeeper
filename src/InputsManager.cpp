#include "pch.h"
#include "InputsManager.h"
#include "ToolsUIManager.h"
#include "GameMain.h"
#include "GuiManager.h"

InputsManager gInputsManager;

bool InputsManager::Initialize()
{
    Clear();

    return true;
}

void InputsManager::Deinit()
{
}

void InputsManager::ProcessInputEvent(MouseButtonInputEvent& inputEvent)
{
    mMouseButtons[inputEvent.mButton] = inputEvent.mPressed;

    if (gToolsUIManager.IsInitialized())
    {
        gToolsUIManager.ProcessInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGuiManager.ProcessInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.ProcessInputEvent(inputEvent);
    }
}

void InputsManager::ProcessInputEvent(MouseMovedInputEvent& inputEvent)
{
    mCursorPosition.x = inputEvent.mCursorPositionX;
    mCursorPosition.y = inputEvent.mCursorPositionY;

    if (gToolsUIManager.IsInitialized())
    {
        gToolsUIManager.ProcessInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGuiManager.ProcessInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.ProcessInputEvent(inputEvent);
    }
}

void InputsManager::ProcessInputEvent(MouseScrollInputEvent& inputEvent)
{
    if (gToolsUIManager.IsInitialized())
    {
        gToolsUIManager.ProcessInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGuiManager.ProcessInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.ProcessInputEvent(inputEvent);
    }
}

void InputsManager::ProcessInputEvent(KeyInputEvent& inputEvent)
{
    mKeyboardKeys[inputEvent.mKeycode] = inputEvent.mPressed;

    if (gToolsUIManager.IsInitialized())
    {
        gToolsUIManager.ProcessInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGuiManager.ProcessInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.ProcessInputEvent(inputEvent);
    }
}

void InputsManager::ProcessInputEvent(KeyCharEvent& inputEvent)
{
    if (gToolsUIManager.IsInitialized())
    {
        gToolsUIManager.ProcessInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGuiManager.ProcessInputEvent(inputEvent);
    }

    if (!inputEvent.mConsumed)
    {
        gGameMain.ProcessInputEvent(inputEvent);
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
