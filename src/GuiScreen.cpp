#include "pch.h"
#include "GuiScreen.h"
#include "GuiWidget.h"
#include "GuiManager.h"

GuiScreen::~GuiScreen()
{
    if (IsScreenActive())
    {
        SetScreenAttached(false);
    }
    mHier.Cleanup();
}

void GuiScreen::RenderFrame(GuiRenderer& renderContext)
{
    if (!IsScreenInitialized())
    {
        debug_assert(false);
        return;
    }
    mHier.RenderFrame(renderContext);
    HandleRenderScreen(renderContext);
}

void GuiScreen::UpdateFrame()
{
    if (!IsScreenInitialized())
    {
        debug_assert(false);
        return;
    }
    mHier.UpdateFrame();
    HandleUpdateScreen();
}

void GuiScreen::CleanupScreen()
{
    if (!IsScreenInitialized())
        return;

    DeactivateScreen();

    HandleCleanupScreen();
    mHier.Cleanup();
    mIsInitialized = false;
}

bool GuiScreen::InitializeScreen()
{
    if (IsScreenInitialized())
        return true;

    mIsInitialized = HandleInitializeScreen();
    debug_assert(mIsInitialized);

    if (!mIsInitialized)
    {
        CleanupScreen();
        return false;
    }
    return true;
}

bool GuiScreen::ActivateScreen()
{
    if (IsScreenActive())
        return true;

    if (!IsScreenInitialized() && !InitializeScreen())
    {
        debug_assert(false);
        return false;
    }

    SetScreenAttached(true);
    HandleStartScreen();

    return true;
}

void GuiScreen::DeactivateScreen()
{
    if (!IsScreenActive())
        return;

    SetScreenAttached(false);
    HandleEndScreen();
}

void GuiScreen::SetScreenAttached(bool isActive)
{
    if (isActive)
    {
        gGuiManager.AttachScreen(this);
    }
    else
    {
        gGuiManager.DetachScreen(this);
    }
    mIsActive = isActive;
}