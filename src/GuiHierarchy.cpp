#include "pch.h"
#include "GuiHierarchy.h"
#include "GuiManager.h"
#include "GuiRenderer.h"
#include "TimeManager.h"

GuiHierarchy::~GuiHierarchy()
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
