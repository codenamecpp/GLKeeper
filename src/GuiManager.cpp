#include "pch.h"
#include "GuiManager.h"
#include "GuiRenderer.h"

GuiManager gGuiManager;

bool GuiManager::Initialize()
{
    return true;
}

void GuiManager::Deinit()
{
}

void GuiManager::RenderFrame(GuiRenderer& renderContext)
{

}

void GuiManager::UpdateFrame()
{
}

void GuiManager::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void GuiManager::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void GuiManager::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void GuiManager::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void GuiManager::HandleInputEvent(KeyCharEvent& inputEvent)
{
}