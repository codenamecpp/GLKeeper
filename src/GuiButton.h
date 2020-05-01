#pragma once

#include "GuiWidget.h"

// button widget
class GuiButton: public GuiWidget
{
public:
    
    static GuiWidgetMetaClass MetaClass; // button widget class

public:
    GuiButton();

protected:
    GuiButton(GuiWidgetMetaClass* widgetClass);
    GuiButton(GuiButton* copyWidget);

    // override GuiWidget
    void HandleRender(GuiRenderer& renderContext) override;
    void HandleInputEvent(MouseButtonInputEvent& inputEvent) override;
    GuiButton* CreateClone() override;

protected:
};
