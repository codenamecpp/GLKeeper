#include "pch.h"
#include "LayoutsEditGamestate.h"
#include "GameMain.h"
#include "GuiManager.h"
#include "GuiButton.h"
#include "GuiPictureBox.h"
#include "GraphicsDevice.h"
#include "GuiPanel.h"
#include "GuiSlider.h"
#include "TexturesManager.h"
#include "ToolsUIManager.h"

void LayoutsEditGamestate::HandleGamestateEnter()
{
    gToolsUIManager.AttachWindow(&mLayoutsEditWindow);
}

void LayoutsEditGamestate::HandleGamestateLeave()
{
    gToolsUIManager.DetachWindow(&mLayoutsEditWindow);
}

void LayoutsEditGamestate::HandleUpdateFrame()
{
}

void LayoutsEditGamestate::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void LayoutsEditGamestate::HandleInputEvent(KeyCharEvent& inputEvent)
{
}