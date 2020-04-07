#pragma once

#include "GuiWidget.h"

// button widget
class GuiButton: public GuiWidget
{
public:
    GuiButton();
    GuiButton(GuiWidgetClass* widgetClass);

protected:
    GuiButton(GuiButton* copyWidget);

    // override GuiWidget
    void HandleRender(GuiRenderer& renderContext) override;
    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;
    bool HasAttribute(eGuiWidgetAttribute attribute) const override;
    GuiButton* ConstructClone() override;

protected:
};

// button widget class
extern GuiWidgetClass gButtonWidgetClass;
