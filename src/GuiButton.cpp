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
    mHasInteractiveAttribute = true;
    mHasSelectableAttribute = true;
}

GuiButton::GuiButton(GuiButton* copyWidget)
    : GuiWidget(copyWidget)
{
}

GuiButton* GuiButton::CreateClone()
{
    GuiButton* selfClone = new GuiButton(this);
    return selfClone;
}
