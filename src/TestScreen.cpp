#include "stdafx.h"
#include "TestScreen.h"
#include "UiHelpers.h"
#include "UiWidgetManager.h"
#include "UiRenderContext.h"
#include "GameMain.h"

//////////////////////////////////////////////////////////////////////////

static const std::string uiscreen_json_path = "ui/test_screen.json";

//////////////////////////////////////////////////////////////////////////

bool TestScreen::LoadContent()
{
    if (!UiView::LoadContent())
    {
        if (mHierarchy.LoadFrom(uiscreen_json_path))
        {

        }
        else
        {
            cxx_assert(false);
        }
    }
    return IsHierarchyLoaded();
}

void TestScreen::Cleanup()
{
    UiView::Cleanup();
}

void TestScreen::UpdateFrame(float deltaTime)
{
    UiView::UpdateFrame(deltaTime);
}

void TestScreen::RenderFrame(UiRenderContext& renderContext)
{
    renderContext.FillRect(renderContext.GetScreenRect(), COLOR_DARK_GRAY);
    UiView::RenderFrame(renderContext);
}

void TestScreen::InputEvent(KeyInputEvent& inputEvent)
{
    if (inputEvent.mPressed)
    {
        // reload screen
        if (inputEvent.mKeycode == KEYCODE_R && inputEvent.mMods == KEYMOD_CTRL)
        {
            inputEvent.SetConsumed();
            Deactivate();
            Cleanup();
            LoadContent();
            Activate();
            return;
        }
    }
}

void TestScreen::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{

}
