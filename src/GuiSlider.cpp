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
    mSliderWidget->SetSize(Point(30, mCurrentSize.y - 20));

    GuiAnchors anchors;
    anchors.mL = true;
    anchors.mR = true;
    anchors.mT = true;
    anchors.mB = true;

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
        case eGuiWidgetAttribute_DragDrop:
        case eGuiWidgetAttribute_PickTarget:
            return true;
    }
    return false;
}

void GuiSlider::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (inputEvent.HasPressed(eMouseButton_Left))
    {
        gGuiManager.SetDragHandler(this, gInputsManager.mCursorPosition);
    }
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
        if (IsBeingDragged()) // force cancel drag
        {
            gGuiManager.SetDragHandler(nullptr, Point());
        }

        mSliderWidget = nullptr;
    }
}

GuiSlider* GuiSlider::ConstructClone()
{
    GuiSlider* selfClone = new GuiSlider(this);
    return selfClone;
}

void GuiSlider::HandleDragStart(const Point& screenPoint)
{
    debug_assert(mSliderWidget);
}

void GuiSlider::HandleDragCancel()
{
    debug_assert(mSliderWidget);
}

void GuiSlider::HandleDragDrop(const Point& screenPoint)
{
    debug_assert(mSliderWidget);
}

void GuiSlider::HandleDrag(const Point& screenPoint)
{
    debug_assert(mSliderWidget);

    Point cursorPositionLocal = ScreenToLocal(screenPoint);

    // map cursor position to size
    int sliderPositionLocal = glm::clamp(cursorPositionLocal.x, 0, mCurrentSize.x);

    mSliderWidget->SetPositionX(sliderPositionLocal);
}