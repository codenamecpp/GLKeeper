#include "pch.h"
#include "GuiHierarchy.h"
#include "GuiManager.h"
#include "GuiRenderer.h"
#include "TimeManager.h"

GuiHierarchy::~GuiHierarchy()
{
    Clear();
}

void GuiHierarchy::Clear()
{
    SafeDelete(mRootWidget);
}

void GuiHierarchy::RenderFrame(GuiRenderer& renderContext)
{
    if (mRootWidget)
    {
        mRootWidget->RenderFrame(renderContext);
    }
}

void GuiHierarchy::UpdateFrame()
{
    if (mRootWidget)
    {
        float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();
        mRootWidget->UpdateFrame(deltaTime);
    }
}

void GuiHierarchy::SetVisible(bool isVisible)
{
    if (mRootWidget)
    {
        mRootWidget->SetVisible(isVisible);
    }
}

void GuiHierarchy::SetEnabled(bool isEnabled)
{
    if (mRootWidget)
    {
        mRootWidget->SetEnabled(isEnabled);
    }
}