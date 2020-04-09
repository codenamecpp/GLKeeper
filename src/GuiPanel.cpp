#include "pch.h"
#include "GuiPanel.h"
#include "GuiManager.h"
#include "InputsManager.h"

// widget class factory
static GuiWidgetFactory<GuiPanel> _PanelWidgetsFactory;

GuiWidgetClass gPanelWidgetClass("panel", &_PanelWidgetsFactory);

GuiPanel::GuiPanel(): GuiPanel(&gPanelWidgetClass)
{
}

GuiPanel::GuiPanel(GuiWidgetClass* widgetClass)
    : GuiWidget(widgetClass)
{
}

GuiPanel::GuiPanel(GuiPanel* copyWidget)
    : GuiWidget(copyWidget)
{
}

void GuiPanel::HandleRender(GuiRenderer& renderContext)
{
}

void GuiPanel::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

bool GuiPanel::HasAttribute(eGuiWidgetAttribute attribute) const
{
    switch (attribute)
    {
        case eGuiWidgetAttribute_Interactive:
            return true;

        case eGuiWidgetAttribute_DragDrop:
            return false;
    }
    return false;
}

GuiPanel* GuiPanel::ConstructClone()
{
    GuiPanel* selfCopy = new GuiPanel(this);
    return selfCopy;
}