#include "pch.h"
#include "GuiSlider.h"
#include "GuiManager.h"
#include "InputsManager.h"
#include "GuiRenderer.h"

// widget class factory
static GuiWidgetFactory<GuiSlider> _SliderWidgetsFactory;

GuiWidgetClass gSliderWidgetClass("slider", &_SliderWidgetsFactory, &gBaseWidgetClass);

GuiSlider::GuiSlider(): GuiSlider(&gSliderWidgetClass)
{
}

GuiSlider::GuiSlider(GuiWidgetClass* widgetClass)
    : GuiWidget(widgetClass)
{
}

GuiSlider::GuiSlider(GuiSlider* copyWidget)
    : GuiWidget(copyWidget)
{
}

void GuiSlider::SetMin(int minValue)
{
    mMin = minValue;
}

void GuiSlider::SetMax(int maxValue)
{
    mMax = maxValue;
}

void GuiSlider::SetPageSize(int pageSize)
{
    mPageSize = pageSize;
}

void GuiSlider::SetupSliderWidget()
{
    debug_assert(mSliderWidget);

    mSliderWidget->SetPosition(Point(50, 50), eGuiUnits_Percents, eGuiUnits_Percents);
    mSliderWidget->SetOrigin(Point(50, 50), eGuiUnits_Percents, eGuiUnits_Percents);
    mSliderWidget->SetSize(Point(80, mSize.y - 20));

    GuiAnchors anchors;
    anchors.mL = true;
    anchors.mR = false;
    anchors.mT = true;
    anchors.mB = false;

    mSliderWidget->SetAnchors(anchors);
}

void GuiSlider::HandleRender(GuiRenderer& renderContext)
{
    Rectangle rc = GetLocalRect();
    renderContext.FillRect(rc, Color32_White);
}

bool GuiSlider::HasAttribute(eGuiWidgetAttribute attribute) const
{
    switch (attribute)
    {
        case eGuiWidgetAttribute_Interactive:
            return true;
    }
    return false;
}

void GuiSlider::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void GuiSlider::HandleChildAttached(GuiWidget* childWidget)
{
    if ((mSliderWidget == nullptr) && (childWidget->mName == "#slider"))
    {
        mSliderWidget = childWidget;
        SetupSliderWidget();
    }
}

void GuiSlider::HandleChildDetached(GuiWidget* childWidget)
{
    if (mSliderWidget && (mSliderWidget == childWidget))
    {

        mSliderWidget = nullptr;
    }
}

GuiSlider* GuiSlider::ConstructClone()
{
    GuiSlider* selfClone = new GuiSlider(this);
    return selfClone;
}
