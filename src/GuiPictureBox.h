#pragma once

#include "GuiWidget.h"

// picture box widget
class GuiPictureBox: public GuiWidget
{
public:
    GuiPictureBox();
    GuiPictureBox(GuiWidgetClass* widgetClass);

protected:
    GuiPictureBox(GuiPictureBox* copyWidget);

    // override GuiWidget
    GuiPictureBox* ConstructClone() override;
};

// picturebox widget class
extern GuiWidgetClass gPictureBoxWidgetClass;