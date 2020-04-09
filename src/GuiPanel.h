#pragma once

#include "GuiWidget.h"

// panel widget
class GuiPanel: public GuiWidget
{
public:
    GuiPanel();

protected:
    GuiPanel(GuiWidgetClass* widgetClass);
    GuiPanel(GuiPanel* copyWidget);

    // override GuiWidget
    void HandleRender(GuiRenderer& renderContext) override;
    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;
    bool HasAttribute(eGuiWidgetAttribute attribute) const override;
    GuiPanel* ConstructClone() override;

protected:
};

// panel widget class
extern GuiWidgetClass gPanelWidgetClass;