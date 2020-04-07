#include "pch.h"
#include "GuiButton.h"

// widget class factory
static GuiWidgetFactory<GuiButton> _ButtonWidgetsFactory;

GuiWidgetClass gButtonWidgetClass("button", &_ButtonWidgetsFactory);

//////////////////////////////////////////////////////////////////////////

GuiButton::GuiButton(): GuiButton(&gButtonWidgetClass)
{
}

GuiButton::GuiButton(GuiWidgetClass* widgetClass)
    : GuiWidget(widgetClass)
{
}

GuiButton::GuiButton(GuiButton* copyWidget)
    : GuiWidget(copyWidget)
{
}

void GuiButton::HandleRender(GuiRenderer& renderContext)
{
}

GuiButton* GuiButton::ConstructClone()
{
    GuiButton* selfCopy = new GuiButton(this);
    return selfCopy;
}