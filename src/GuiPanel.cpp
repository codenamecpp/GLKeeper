#include "pch.h"
#include "GuiPanel.h"
#include "GuiManager.h"
#include "InputsManager.h"
#include "GuiRenderer.h"

// widget class factory
static GuiWidgetFactory<GuiPanel> _PanelWidgetsFactory;
GuiWidgetMetaClass GuiPanel::MetaClass(cxx::unique_string("panel"), &_PanelWidgetsFactory, &GuiWidget::MetaClass);

GuiPanel::GuiPanel(): GuiPanel(&MetaClass)
{
}

GuiPanel::GuiPanel(GuiWidgetMetaClass* widgetClass)
    : GuiWidget(widgetClass)
{
}

GuiPanel::GuiPanel(GuiPanel* copyWidget)
    : GuiWidget(copyWidget)
{
}

void GuiPanel::HandleRender(GuiRenderer& renderContext)
{
    Rectangle rcLocal = GetLocalRect();

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
    }
    return false;
}

GuiPanel* GuiPanel::ConstructClone()
{
    GuiPanel* selfClone = new GuiPanel(this);
    return selfClone;
}