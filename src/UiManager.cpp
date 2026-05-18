#include "stdafx.h"
#include "UiManager.h"
#include "UiWidgetManager.h"
#include "FontManager.h"
#include "ToolsUiManager.h"

//////////////////////////////////////////////////////////////////////////

UiManager gUiManager;

//////////////////////////////////////////////////////////////////////////

bool UiManager::Initialize()
{
    if (!gWidgetManager.Initialize())
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot initialize ui system");
        return false;
    }

    if (!gToolsUiManager.Initialize())
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot initialize imgui");
        cxx_assert(false);
    }

    PreloadBaseFonts();
    return true;
}

void UiManager::Shutdown()
{
    gToolsUiManager.Shutdown();
    gWidgetManager.Shutdown();
}

void UiManager::UpdateFrame(float deltaTime)
{
    gToolsUiManager.UpdateFrame(deltaTime);

    gWidgetManager.UpdateFrame(deltaTime);
}

void UiManager::InputEvent(KeyInputEvent& inputEvent)
{
    if (gToolsUiManager.IsInitialized())
    {
        gToolsUiManager.InputEvent(inputEvent);
    }

    gWidgetManager.InputEvent(inputEvent);
}

void UiManager::InputEvent(KeyCharEvent& inputEvent)
{
    if (gToolsUiManager.IsInitialized())
    {
        gToolsUiManager.InputEvent(inputEvent);
    }

    gWidgetManager.InputEvent(inputEvent);
}

void UiManager::InputEvent(MouseButtonInputEvent& inputEvent)
{
    if (gToolsUiManager.IsInitialized())
    {
        gToolsUiManager.InputEvent(inputEvent);
    }

    gWidgetManager.InputEvent(inputEvent);
}

void UiManager::InputEvent(MouseMovedInputEvent& inputEvent)
{
    if (gToolsUiManager.IsInitialized())
    {
        gToolsUiManager.InputEvent(inputEvent);
    }

    gWidgetManager.InputEvent(inputEvent);
}

void UiManager::InputEvent(MouseScrollInputEvent& inputEvent)
{
    if (gToolsUiManager.IsInitialized())
    {
        gToolsUiManager.InputEvent(inputEvent);
    }

    gWidgetManager.InputEvent(inputEvent);
}

Font* UiManager::GetDefaultFont(eDefaultFont id) const
{
    switch (id)
    {
        case eDefaultFont::FpsBlock: 
            return mFpsFont;
        case eDefaultFont::Debug:
        case eDefaultFont::Console:
            return mConsoleFont;
    }
    cxx_assert(false);
    return mConsoleFont;
}

void UiManager::ScreenSizeChanged()
{
    gWidgetManager.ScreenSizeChanged();
}

void UiManager::PreloadBaseFonts()
{
    mConsoleFont = gFontManager.GetFont("console_font"); 
    cxx_assert(mConsoleFont);

    mFpsFont = gFontManager.GetFont("fps_font");
    cxx_assert(mFpsFont);
}
