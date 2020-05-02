#include "pch.h"
#include "GuiTestGamestate.h"
#include "GameMain.h"
#include "GuiManager.h"
#include "GuiButton.h"
#include "GuiPictureBox.h"
#include "GraphicsDevice.h"
#include "GuiPanel.h"
#include "GuiScrollbar.h"
#include "TexturesManager.h"
#include "ToolsUIManager.h"
#include "Console.h"

GuiTestGamestate::GuiTestGamestate()
{
}

void GuiTestGamestate::HandleGamestateEnter()
{
    if (!mScreen.ActivateScreen())
    {
        debug_assert(false);
    }
}

void GuiTestGamestate::HandleGamestateLeave()
{
    mScreen.DeactivateScreen();
}

void GuiTestGamestate::HandleUpdateFrame()
{
}

void GuiTestGamestate::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void GuiTestGamestate::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
}

void GuiTestGamestate::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void GuiTestGamestate::HandleInputEvent(KeyInputEvent& inputEvent)
{
    if (inputEvent.HasPressed(eKeycode_F5))
    {
        mScreen.DeactivateScreen();
        mScreen.CleanupScreen();
        if (mScreen.InitializeScreen())
        {
            mScreen.ActivateScreen();
        }
        else
        {
            debug_assert(false);
        }
    }
}

void GuiTestGamestate::HandleInputEvent(KeyCharEvent& inputEvent)
{
}