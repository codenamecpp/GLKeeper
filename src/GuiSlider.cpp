#include "pch.h"
#include "GuiSlider.h"
#include "GuiManager.h"
#include "InputsManager.h"
#include "GuiRenderer.h"
#include "Console.h"

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

void GuiSlider::SetupControlWidget(GuiWidget* controlWidget)
{
    debug_assert(controlWidget);
    if (controlWidget == mSliderWidget)
    {
        controlWidget->SetPosition(Point(50, 50), eGuiUnits_Percents, eGuiUnits_Percents);
        controlWidget->SetOrigin(Point(50, 50), eGuiUnits_Percents, eGuiUnits_Percents);
        controlWidget->SetSize(Point(80, mSize.y - 20));

        GuiAnchors anchors;
        anchors.mL = true;
        anchors.mR = false;
        anchors.mT = true;
        anchors.mB = false;

        controlWidget->SetAnchors(anchors);
    }

    if (controlWidget == mMinWidget)
    {
        controlWidget->SetPosition(Point(0, 50), eGuiUnits_Percents, eGuiUnits_Percents);
        controlWidget->SetOrigin(Point(50, 50), eGuiUnits_Percents, eGuiUnits_Percents);
        controlWidget->SetSize(Point(mSize.y - 20, mSize.y - 20));

        GuiAnchors anchors;
        anchors.mL = true;
        anchors.mR = false;
        anchors.mT = true;
        anchors.mB = false;

        controlWidget->SetAnchors(anchors);
    }

    if (controlWidget == mMaxWidget)
    {
        controlWidget->SetPosition(Point(100, 50), eGuiUnits_Percents, eGuiUnits_Percents);
        controlWidget->SetOrigin(Point(50, 50), eGuiUnits_Percents, eGuiUnits_Percents);
        controlWidget->SetSize(Point(mSize.y - 20, mSize.y - 20));

        GuiAnchors anchors;
        anchors.mL = true;
        anchors.mR = false;
        anchors.mT = true;
        anchors.mB = false;

        controlWidget->SetAnchors(anchors);
    }
}

void GuiSlider::HandleUpdate(float deltaTime)
{
    if (mSliderWidget && mSliderWidget->IsMouseCaptured())
    {
        Point pos = mSliderWidget->ScreenToLocal(gInputsManager.mCursorPosition) + mSliderWidget->GetPosition() - mSliderWidget->GetOrigin();
        mSliderWidget->SetPositionX(pos.x);
    }
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
    bool doSubscribe = true;
    if ((mSliderWidget == nullptr) && (childWidget->mName == "#slider"))
    {
        mSliderWidget = childWidget;
        doSubscribe = true;
    }
    if ((mMinWidget == nullptr) && (childWidget->mName == "#min"))
    {
        mMinWidget = childWidget;
        doSubscribe = true;
    }
    if ((mMaxWidget == nullptr) && (childWidget->mName == "#max"))
    {
        mMaxWidget = childWidget;
        doSubscribe = true;
    }

    if (doSubscribe)
    {
        Subscribe(childWidget, eGuiEvent_MouseDown);
        Subscribe(childWidget, eGuiEvent_MouseUp);

        SetupControlWidget(childWidget);
    }
}

void GuiSlider::HandleChildDetached(GuiWidget* childWidget)
{
    if (mSliderWidget && (mSliderWidget == childWidget))
    {
        mSliderWidget = nullptr;
    }

    if (mMinWidget && (mMinWidget == childWidget))
    {
        mMinWidget = nullptr;
    }

    if (mMaxWidget && (mMaxWidget == childWidget))
    {
        mMaxWidget = nullptr;
    }

    Unsubscribe(childWidget);
}

GuiSlider* GuiSlider::ConstructClone()
{
    GuiSlider* selfClone = new GuiSlider(this);
    return selfClone;
}

void GuiSlider::HandleMouseDown(GuiWidget* sender, eMouseButton mbutton)
{
    if (sender == mMinWidget)
    {
        mSliderWidget->SetPositionX(mSliderWidget->GetPosition().x - 10);
    }

    if (sender == mMaxWidget)
    {
        mSliderWidget->SetPositionX(mSliderWidget->GetPosition().x + 10);
    }

    if (sender == mSliderWidget)
    {
    }
}

void GuiSlider::HandleMouseUp(GuiWidget* sender, eMouseButton mbutton)
{
    if (sender == mMinWidget)
    {

    }

    if (sender == mMaxWidget)
    {

    }

    if (sender == mSliderWidget)
    {

    }
}