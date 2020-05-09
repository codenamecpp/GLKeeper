#include "pch.h"
#include "GuiTestScreen.h"
#include "GuiManager.h"
#include "Console.h"
#include "GuiButton.h"

GuiTestScreen::GuiTestScreen()
    : GuiScreen(cxx::unique_string("gui_test_screen"))
{
    SetScreenInteractable(false);
    SetScreenModal(true);
}

bool GuiTestScreen::ReloadScreen()
{
    CleanupScreen();
    if (ShowScreen())
        return true;

    debug_assert(false);
    return false;
}

void GuiTestScreen::HandleScreenRender(GuiRenderer& renderContext)
{
}

void GuiTestScreen::HandleScreenUpdate()
{
}

void GuiTestScreen::HandleScreenCleanup()
{
}

void GuiTestScreen::HandleScreenLoad()
{
}

void GuiTestScreen::HandleScreenShow()
{
}

void GuiTestScreen::HandleScreenHide()
{
}

void GuiTestScreen::HandleClick(GuiWidget* sender, eMouseButton mbuton)
{
}

void GuiTestScreen::HandlePressStart(GuiWidget* sender, eMouseButton mbuton)
{
}

void GuiTestScreen::HandlePressEnd(GuiWidget* sender, eMouseButton mbuton)
{
}

void GuiTestScreen::HandleMouseEnter(GuiWidget* sender)
{
}

void GuiTestScreen::HandleMouseLeave(GuiWidget* sender)
{
}

void GuiTestScreen::HandleMouseDown(GuiWidget* sender, eMouseButton mbutton)
{
}

void GuiTestScreen::HandleMouseUp(GuiWidget* sender, eMouseButton mbutton)
{
}

void GuiTestScreen::HandleEvent(GuiWidget* sender, cxx::unique_string eventId)
{
}

bool GuiTestScreen::ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue)
{
    return false;
}
