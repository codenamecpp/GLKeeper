#include "stdafx.h"
#include "FrontendScreen.h"
#include "FrontendController.h"
#include "GameMain.h"
#include "Version.h"
#include "UiTextBox.h"

//////////////////////////////////////////////////////////////////////////

static const std::string uiscreen_json_path = "ui/frontend_screen.json";

//////////////////////////////////////////////////////////////////////////

enum eMainMenuNotifictionId
{
    eMainMenuNotifictionId_None = 0,

    eMainMenuNotifictionId_SinglePlayer = 100,
    eMainMenuNotifictionId_Quit,
};

//////////////////////////////////////////////////////////////////////////

FrontendScreen::FrontendScreen(FrontendController& frontend)
    : mFrontend(frontend)
{

}

bool FrontendScreen::LoadContent()
{
    if (!UiView::LoadContent())
    {
        if (mHierarchy.LoadFrom(uiscreen_json_path))
        {
            if (UiWidget* uiWidget = mHierarchy.FindWidgetWithName("single_player"))
            {
                uiWidget->Subscribe(this);
                uiWidget->UserData().SetValue(eMainMenuNotifictionId_SinglePlayer);
            }

            if (UiWidget* uiWidget = mHierarchy.FindWidgetWithName("quit"))
            {
                uiWidget->Subscribe(this);
                uiWidget->UserData().SetValue(eMainMenuNotifictionId_Quit);
            }

            if (UiWidget* uiWidget = mHierarchy.FindWidgetWithName("version_string"))
            {
                UiTextBox* textBox = (UiTextBox*) uiWidget;

                // set version string
                std::wstring versionNumber {GAME_VERSION_STRING, GAME_VERSION_STRING + sizeof(GAME_VERSION_STRING)};
                versionNumber.insert(versionNumber.begin(), L'V');
                textBox->SetText(versionNumber);
            }
        }
        else
        {
            cxx_assert(false);
        }
    }
    return IsHierarchyLoaded();
}

void FrontendScreen::Cleanup()
{
    UiView::Cleanup();
}

void FrontendScreen::InputEvent(KeyInputEvent& inputEvent)
{

}

void FrontendScreen::UpdateFrame(float deltaTime)
{

}

void FrontendScreen::OnActivated()
{

}

void FrontendScreen::OnDeactivated()
{

}

void FrontendScreen::HandleUiEvent(UiWidget* sender, const UiEventDesc* eventDesc)
{
    eMainMenuNotifictionId notificationId = sender->UserData().GetValue<eMainMenuNotifictionId>();

    if (notificationId == eMainMenuNotifictionId_Quit)
    {
        mFrontend.OnQuitGameSelected();
        return;
    }

    if (notificationId == eMainMenuNotifictionId_SinglePlayer)
    {
        mFrontend.OnStartSinglePlayerGameSelected();
        return;
    }
}
