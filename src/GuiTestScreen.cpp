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
    GuiWidget* featureButtonsLits = mHier.SearchForWidget("feature_buttons_list");
    debug_assert(featureButtonsLits);

    static cxx::unique_string featureButtonTemplateName("test_button");
    if (featureButtonsLits)
    {
        int NumFeatureButtons = 12;
        mFeatureButtons.resize(NumFeatureButtons);

        for (int icurr = 0; icurr < NumFeatureButtons; ++icurr)
        {
            GuiWidget* featureButtonWidget = mHier.CreateTemplateWidget(featureButtonTemplateName);
            debug_assert(featureButtonWidget);
            featureButtonsLits->AttachChild(featureButtonWidget);

            mFeatureButtons[icurr].SetActiveState(false);
            mFeatureButtons[icurr].SetAvailableState(true);
            mFeatureButtons[icurr].SetControl(featureButtonWidget);

            Subscribe(GuiEventId_OnClick, featureButtonWidget);
            Subscribe(GuiEventId_OnPressStart, featureButtonWidget);
        }
    }
}

void GuiTestScreen::HandleEndScreen()
{
    UnsubscribeAll();
    mFeatureButtons.clear();
}

void GuiTestScreen::HandleClick(GuiWidget* sender, eMouseButton mbuton)
{
    gConsole.LogMessage(eLogMessage_Debug, "event: click (%s)", cxx::enum_to_string(mbuton));
}

void GuiTestScreen::HandlePressStart(GuiWidget* sender, eMouseButton mbuton)
{
    gConsole.LogMessage(eLogMessage_Debug, "event: press start (%s)", cxx::enum_to_string(mbuton));

    if (mbuton != eMouseButton_Left)
        return;

    for (GameUiFeatureButton& currFeatureButton: mFeatureButtons)
    {
        currFeatureButton.SetActiveState(sender == currFeatureButton.mControl);
    }
}

void GuiTestScreen::HandlePressEnd(GuiWidget* sender, eMouseButton mbuton)
{
    gConsole.LogMessage(eLogMessage_Debug, "event: press end (%s)", cxx::enum_to_string(mbuton));
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
    gConsole.LogMessage(eLogMessage_Debug, "event: %s", eventId.c_str());
}

bool GuiTestScreen::ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue)
{
    return false;
}
