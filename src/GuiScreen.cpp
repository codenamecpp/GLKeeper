#include "pch.h"
#include "GuiScreen.h"
#include "GuiWidget.h"
#include "GuiManager.h"
#include "GraphicsDevice.h"

GuiScreen::GuiScreen(const std::string& screenId, const std::string& contentPath)
    : mScreenId(screenId)
    , mContentPath(contentPath)
{
    debug_assert(!mContentPath.empty());
}

GuiScreen::~GuiScreen()
{
    UnsubscribeAll();
    SetScreenAttached(false);
    mHier.Cleanup();
}

void GuiScreen::RenderFrame(GuiRenderer& renderContext)
{
    if (!IsScreenLoaded() || !IsScreenShown())
        return;

    mHier.RenderFrame(renderContext);
    HandleScreenRender(renderContext);
}

void GuiScreen::UpdateFrame()
{
    if (!IsScreenLoaded())
        return;

    mHier.UpdateFrame();
    HandleScreenUpdate();
}

void GuiScreen::SetScreenAttached(bool isAttached)
{
    if (isAttached)
    {
        gGuiManager.AttachScreen(this);
    }
    else
    {
        gGuiManager.DetachScreen(this);
    }
}

void GuiScreen::CleanupScreen()
{
    if (!IsScreenLoaded())
        return;

    UnsubscribeAll();
    SetScreenAttached(false);
    HandleScreenCleanup();
    mHier.Cleanup();
}

bool GuiScreen::LoadScreen()
{
    if (IsScreenLoaded())
        return true;

    bool isLoaded = mHier.LoadFromFile(mContentPath);
    if (!isLoaded)
    {
        debug_assert(false);
        return false;
    }

    mHier.FitLayoutToScreen(gGraphicsDevice.mScreenResolution);
    mHier.SetVisible(false);

    SetScreenAttached(true);   
    HandleScreenLoad();
    return true;
}

bool GuiScreen::ShowScreen()
{
    if (IsScreenShown())
        return true;

    if (!IsScreenLoaded() && !LoadScreen())
    {
        debug_assert(false);
        return false;
    }
    mHier.SetVisible(true);
    HandleScreenShow();
    return true;
}

void GuiScreen::HideScreen()
{
    if (!IsScreenShown())
        return;

    mHier.SetVisible(false);
    HandleScreenHide();
}

bool GuiScreen::IsScreenLoaded() const
{
    if (mHier.mRootWidget)
        return true;

    return false;
}

bool GuiScreen::IsScreenShown() const
{
    return mHier.mRootWidget && mHier.mRootWidget->IsVisible();
}
