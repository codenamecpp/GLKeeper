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
    mHasInteractiveAttribute = true;
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

GuiPanel* GuiPanel::CreateClone()
{
    GuiPanel* selfClone = new GuiPanel(this);
    return selfClone;
}