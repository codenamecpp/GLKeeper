#include "pch.h"
#include "GuiScreenLayer.h"
#include "GuiWidget.h"

GuiScreenLayer::GuiScreenLayer()
{
}

GuiScreenLayer::~GuiScreenLayer()
{
    mHier.Cleanup();
}

void GuiScreenLayer::RenderFrame(GuiRenderer& renderContext)
{
    HandlePreRenderHier(renderContext);
    mHier.RenderFrame(renderContext);
    HandlePostRenderHier(renderContext);
}

void GuiScreenLayer::UpdateFrame()
{
    HandlePreUpdateHier();
    mHier.UpdateFrame();
    HandlePostUpdateHier();
}