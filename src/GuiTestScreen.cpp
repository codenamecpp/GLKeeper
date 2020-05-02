#include "pch.h"
#include "GuiTestScreen.h"
#include "GuiManager.h"
#include "Console.h"

bool GuiTestScreen::ReloadScreen()
{
    DeactivateScreen();
    CleanupScreen();
    if (InitializeScreen())
    {
        ActivateScreen();
        return true;
    }

    debug_assert(false);
    return false;
}

void GuiTestScreen::HandleRenderScreen(GuiRenderer& renderContext)
{
}

void GuiTestScreen::HandleUpdateScreen()
{
}

void GuiTestScreen::HandleCleanupScreen()
{
    UnsubscribeAll();
}

bool GuiTestScreen::HandleInitializeScreen()
{
    if (!mHier.LoadFromFile("screens/gui_test.json"))
    {
        debug_assert(false);
        return false;
    }
    return true;
}

void GuiTestScreen::HandleStartScreen()
{
    if (GuiWidget* buttonWidget = mHier.SearchForWidget("button_0"))
    {
        Subscribe(GuiEventId_OnClick, buttonWidget);
        Subscribe(GuiEventId_OnMouseDown, buttonWidget);
        Subscribe(GuiEventId_OnMouseUp, buttonWidget);
        Subscribe(GuiEventId_OnMouseEnter, buttonWidget);
        Subscribe(GuiEventId_OnMouseLeave, buttonWidget);
        Subscribe(cxx::unique_string("custom_on_click_event"), buttonWidget);
        buttonWidget->SetActionsContext(this);
    }

    if (GuiWidget* buttonWidget = mHier.SearchForWidget("button_1"))
    {
        Subscribe(GuiEventId_OnClick, buttonWidget);
        Subscribe(GuiEventId_OnMouseDown, buttonWidget);
        Subscribe(GuiEventId_OnMouseUp, buttonWidget);
        Subscribe(GuiEventId_OnMouseEnter, buttonWidget);
        Subscribe(GuiEventId_OnMouseLeave, buttonWidget);
        Subscribe(cxx::unique_string("custom_on_click_event"), buttonWidget);
        buttonWidget->SetEnabled(false);
        buttonWidget->SetActionsContext(this);
    }

    GuiWidget* sliderThumb = mHier.GetWidgetByPath("root/slider_0/#slider");
    Subscribe(GuiEventId_OnMouseDown, sliderThumb);
}

void GuiTestScreen::HandleEndScreen()
{
    UnsubscribeAll();
}

void GuiTestScreen::HandleClick(GuiWidget* sender)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_click %s", sender->mId.c_str());
}

void GuiTestScreen::HandleMouseEnter(GuiWidget* sender)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_mouse_enter %s", sender->mId.c_str());
}

void GuiTestScreen::HandleMouseLeave(GuiWidget* sender)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_mouse_leave %s", sender->mId.c_str());
}

void GuiTestScreen::HandleMouseDown(GuiWidget* sender, eMouseButton mbutton)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_mouse_down %s", sender->mId.c_str());
}

void GuiTestScreen::HandleMouseUp(GuiWidget* sender, eMouseButton mbutton)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_mouse_up %s", sender->mId.c_str());
}

void GuiTestScreen::HandleEvent(GuiWidget* sender, cxx::unique_string eventId)
{
    debug_assert(sender);
    gConsole.LogMessage(eLogMessage_Debug, "on_event '%s'", eventId.c_str());
}

bool GuiTestScreen::ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue)
{
    return false;
}
