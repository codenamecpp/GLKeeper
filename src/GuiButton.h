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
    GuiButton* ConstructClone() override;

};

// button widget class
extern GuiWidgetClass gButtonWidgetClass;
