#include "pch.h"
#include "GuiPictureBox.h"

// base widget class factory
static GuiWidgetFactory<GuiPictureBox> _PictureBoxWidgetsFactory;

GuiWidgetClass gPictureBoxWidgetClass("picture_box", &_PictureBoxWidgetsFactory);

//////////////////////////////////////////////////////////////////////////

GuiPictureBox::GuiPictureBox() : GuiPictureBox(&gPictureBoxWidgetClass)
{
}

GuiPictureBox::GuiPictureBox(GuiWidgetClass* widgetClass)
{
}

GuiPictureBox::GuiPictureBox(GuiPictureBox* copyWidget)
    : GuiWidget(copyWidget)
{
}


GuiPictureBox* GuiPictureBox::ConstructClone()
{
    GuiPictureBox* selfCopy = new GuiPictureBox(this);
    return selfCopy;
}