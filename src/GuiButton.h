#pragma once

#include "GuiWidget.h"

// button widget
class GuiButton: public GuiWidget
{
public:
    GuiButton();

protected:
    GuiButton(GuiWidgetClass* widgetClass);
    GuiButton(GuiButton* copyWidget);

    // override GuiWidget
    void HandleRender(GuiRenderer& renderContext) override;
    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;
    bool HasAttribute(eGuiWidgetAttribute attribute) const override;
    GuiButton* ConstructClone() override;
    // override drag'n'drop
    void HandleDragDrop(const Point& screenPoint) override;

protected:
};

// button widget class
extern GuiWidgetClass gButtonWidgetClass;
