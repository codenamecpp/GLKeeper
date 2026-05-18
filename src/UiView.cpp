#include "stdafx.h"
#include "UiView.h"
#include "UiWidgetManager.h"

UiView::UiView(eUiViewLayer viewLayer)
    : mViewLayer(viewLayer)
{
}

UiView::~UiView() 
{
    mHierarchy.Cleanup();
}

void UiView::ResolutionChanged()
{
    if (UiWidget* rootWidget = mHierarchy.GetRootWidget())
    {
        const Point2D& screenSize = gRenderDevice.GetScreenResolution();
        rootWidget->FitLayoutToScreen(screenSize);
    }
}

bool UiView::IsActive() const
{
    bool isAttached = gWidgetManager.ViewAttached(this);
    return isAttached;
}

void UiView::RenderFrame(UiRenderContext& renderContext)
{
    if (UiWidget* rootWidget = mHierarchy.GetRootWidget())
    {
        rootWidget->RenderFrame(renderContext);
    }
}

void UiView::UpdateFrame(float deltaTime)
{
    if (UiWidget* rootWidget = mHierarchy.GetRootWidget())
    {
        rootWidget->UpdateFrame(deltaTime);
    }
}

bool UiView::Activate()
{
    if (IsActive())
        return true;

    // force load screen content
    bool isLoaded = IsHierarchyLoaded();
    if (!isLoaded)
    {
        isLoaded = LoadContent();
        cxx_assert(isLoaded);
        if (!isLoaded)
            return false;
    }

    // attach widgets layer to uisystem
    gWidgetManager.AttachView(this);

    OnActivated();
    ResolutionChanged();
    return true;
}

void UiView::Deactivate()
{
    if (!IsActive())
        return;

    // detach widgets layer from uisystem
    gWidgetManager.DetachView(this);
    
    OnDeactivated();
}

bool UiView::LoadContent()
{
    bool isLoaded = IsHierarchyLoaded();
    if (isLoaded)
        return true;

    return false;
}

void UiView::Cleanup()
{
    mHierarchy.Cleanup();
}