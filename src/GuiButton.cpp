#include "pch.h"
#include "GuiButton.h"
#include "GuiRenderer.h"
#include "InputsManager.h"
#include "GuiManager.h"
#include "GuiEvent.h"

// widget class factory
static GuiWidgetFactory<GuiButton> _ButtonWidgetsFactory;
GuiWidgetMetaClass GuiButton::MetaClass(cxx::unique_string("button"), &_ButtonWidgetsFactory, &GuiWidget::MetaClass);

//////////////////////////////////////////////////////////////////////////

GuiButton::GuiButton(): GuiButton(&MetaClass)
{
}

GuiButton::GuiButton(GuiWidgetMetaClass* widgetClass)
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

    Rectangle rc = GetLocalRect();

    Color32 fillColor = mDebugColorNormal;
    if (!IsEnabled())
    {
        fillColor = mDebugColorDisabled;
    }
    else if (IsHovered())
    {
        fillColor = mDebugColorHovered;
    }
    else if (IsSelected())
    {
        fillColor = Color32_Green;
    }
    renderContext.FillRect(rc, fillColor);

#endif
}

void GuiButton::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

bool GuiButton::HasAttribute(eGuiWidgetAttribute attribute) const
{
    switch (attribute)
    {
        case eGuiWidgetAttribute_Selectable:
        case eGuiWidgetAttribute_Interactive:
            return true;
    }
    return false;
}

GuiButton* GuiButton::CreateClone()
{
    GuiButton* selfClone = new GuiButton(this);
    return selfClone;
}
