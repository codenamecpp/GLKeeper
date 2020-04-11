#include "pch.h"
#include "GuiPanel.h"
#include "GuiManager.h"
#include "InputsManager.h"
#include "GuiRenderer.h"

// widget class factory
static GuiWidgetFactory<GuiPanel> _PanelWidgetsFactory;

GuiWidgetClass gPanelWidgetClass("panel", &_PanelWidgetsFactory, &gBaseWidgetClass);

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
    Rectangle rcLocal;
    GetLocalRect(rcLocal);

    Color32 fillColor = Color32_SkyBlue;
    fillColor.mA = 128;
    renderContext.FillRect(rcLocal, fillColor);
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
    GuiPanel* selfClone = new GuiPanel(this);
    return selfClone;
}