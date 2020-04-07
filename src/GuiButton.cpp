#include "pch.h"
#include "GuiButton.h"
#include "GuiRenderer.h"
#include "InputsManager.h"

// widget class factory
static GuiWidgetFactory<GuiButton> _ButtonWidgetsFactory;

GuiWidgetClass gButtonWidgetClass("button", &_ButtonWidgetsFactory);

//////////////////////////////////////////////////////////////////////////

GuiButton::GuiButton(): GuiButton(&gButtonWidgetClass)
{
}

GuiButton::GuiButton(GuiWidgetClass* widgetClass)
    : GuiWidget(widgetClass)
{
}

GuiButton::GuiButton(GuiButton* copyWidget)
    : GuiWidget(copyWidget)
{
}

void GuiButton::HandleRender(GuiRenderer& renderContext)
{
#ifdef _DEBUG

    Rectangle rc;
    GetLocalRect(rc);

    Color32 fillColor = mDebugColorNormal;
    if (!IsEnabled())
    {
        fillColor = mDebugColorDisabled;
    }
    else if (IsHovered())
    {
        fillColor = mDebugColorHovered;
    }
    renderContext.FillRect(rc, fillColor);

#endif
}

void GuiButton::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (inputEvent.HasPressed(eMouseButton_Left))
    {
        StartDrag(gInputsManager.mCursorPosition);
    }
}

bool GuiButton::HasAttribute(eGuiWidgetAttribute attribute) const
{
    switch (attribute)
    {
        case eGuiWidgetAttribute_Interactive:
        case eGuiWidgetAttribute_DragDrop:
            return true;
    }
    return false;
}

GuiButton* GuiButton::ConstructClone()
{
    GuiButton* selfCopy = new GuiButton(this);
    return selfCopy;
}