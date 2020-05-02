#include "pch.h"
#include "GuiTestScreen.h"
#include "GuiManager.h"
#include "Console.h"
#include "GuiButton.h"

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
    GuiButton* buttonWidget = GuiCastWidgetClass<GuiButton>(mHier.SearchForWidget("button_0"));
    if (buttonWidget)
    {
        mFeatureButtonController.SetAvailableState(true);
        mFeatureButtonController.Bind(buttonWidget);
        Subscribe(GuiEventId_OnMouseDown, buttonWidget);
    }
}

void GuiTestScreen::HandleEndScreen()
{
    UnsubscribeAll();
    mFeatureButtonController.Unbind();
}

void GuiTestScreen::HandleClick(GuiWidget* sender)
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
    if (sender == mFeatureButtonController.mBoundButton)
    {
        mFeatureButtonController.ToggleActiveState();
    }
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
