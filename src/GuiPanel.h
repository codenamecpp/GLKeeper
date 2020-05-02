#pragma once

#include "GuiWidget.h"

// panel widget
class GuiPanel: public GuiWidget
{
public:

    static GuiWidgetMetaClass MetaClass; // panel widget class

public:
    GuiPanel();

protected:
    GuiPanel(GuiWidgetMetaClass* widgetClass);
    GuiPanel(GuiPanel* copyWidget);

    // override GuiWidget
    GuiPanel* CreateClone() override;

protected:
};