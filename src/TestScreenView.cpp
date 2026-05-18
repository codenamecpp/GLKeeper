#include "stdafx.h"
#include "TestScreenView.h"
#include "UiHelpers.h"
#include "UiWidgetManager.h"
#include "UiRenderContext.h"
#include "GameMain.h"

//////////////////////////////////////////////////////////////////////////

static const std::string uiscreen_json_path = "ui/test_screen.json";

//////////////////////////////////////////////////////////////////////////

bool TestScreenView::LoadContent()
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

void TestScreenView::Cleanup()
{
    UiView::Cleanup();
}

void TestScreenView::UpdateFrame(float deltaTime)
{

}

void TestScreenView::RenderFrame(UiRenderContext& renderContext)
{
    renderContext.FillRect(renderContext.GetScreenRect(), COLOR_DARK_GRAY);
    UiView::RenderFrame(renderContext);
}

void TestScreenView::InputEvent(KeyInputEvent& inputEvent)
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

void TestScreenView::HandleUiEvent(UiWidget* sender, const UiEventDesc* eventDesc)
{

}
