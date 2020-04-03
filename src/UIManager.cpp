#include "pch.h"
#include "UIManager.h"
#include "UIRenderer.h"

UIManager gUIManager;

bool UIManager::Initialize()
{
    return true;
}

void UIManager::Deinit()
{
}

void UIManager::RenderFrame(UIRenderer& renderContext)
{

}

void UIManager::UpdateFrame()
{
}

void UIManager::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void UIManager::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void UIManager::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void UIManager::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void UIManager::HandleInputEvent(KeyCharEvent& inputEvent)
{
}