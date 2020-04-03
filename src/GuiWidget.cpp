#include "pch.h"
#include "GuiWidget.h"
#include "GuiManager.h"

// base widget class factory
static GuiWidgetFactory<GuiWidget> _BaseWidgetsFactory;

GuiWidgetClass gBaseWidgetClass("widget", &_BaseWidgetsFactory);

GuiWidget::GuiWidget(): GuiWidget(&gBaseWidgetClass)
{
}

GuiWidget::GuiWidget(GuiWidgetClass* widgetClass)
    : mClass(widgetClass)
{
    debug_assert(mClass);
}

GuiWidget::~GuiWidget()
{
}

void GuiWidget::RenderFrame(GuiRenderer& renderContext)
{
}

void GuiWidget::UpdateFrame(float deltaTime)
{
}