#include "pch.h"
#include "GuiButton.h"
#include "GuiRenderer.h"
#include "InputsManager.h"
#include "GuiManager.h"
#include "GuiEvent.h"

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
    else if (IsBeingDragged())
    {
        fillColor = Color32_Green;
    }
    renderContext.FillRect(rc, fillColor);

#endif
}

void GuiButton::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (inputEvent.HasPressed(eMouseButton_Left))
    {
        gGuiManager.SetDragHandler(this, gInputsManager.mCursorPosition);
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

void GuiButton::HandleDragDrop(const Point& screenPoint)
{
    if (IsScreenPointInsideRect(screenPoint))
    {
        GuiEvent ev(this, eGuiEvent_Click);
        gGuiManager.PostGuiEvent(&ev);
    }
}
