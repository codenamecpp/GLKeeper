#include "pch.h"
#include "GuiLayer.h"
#include "GuiManager.h"

GuiLayer::~GuiLayer()
{
    SafeDelete(mRootWidget);
}

void GuiLayer::RenderFrame(GuiRenderer& renderContext)
{
    if (mRootWidget)
    {
        mRootWidget->RenderFrame(renderContext);
    }

    HandleRenderFrame(renderContext);
}

void GuiLayer::UpdateFrame(float deltaTime)
{
    if (mRootWidget)
    {
        mRootWidget->UpdateFrame(deltaTime);
    }

    HandleUpdateFrame();
}