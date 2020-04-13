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
    if (mSliderWidget && controlWidget == mSliderWidget)
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

    if (mDecPosWidget && controlWidget == mDecPosWidget)
    {
        controlWidget->SetPosition(Point(mSpacing, mSpacing));
        controlWidget->SetOrigin(Point(0, 0), eGuiUnits_Percents, eGuiUnits_Percents);
        controlWidget->SetSize(Point(32, 32));

        controlWidget->SetAnchors(GuiAnchors(true, true, false, true));
    }

    if (mIncPosWidget && controlWidget == mIncPosWidget)
    {
        controlWidget->SetPosition(Point(mSize.x - mSpacing, mSpacing));
        controlWidget->SetOrigin(Point(100, 0), eGuiUnits_Percents, eGuiUnits_Percents);
        controlWidget->SetSize(Point(32, 32));

        controlWidget->SetAnchors(GuiAnchors(false, true, true, true));
    }
}

void GuiSlider::HandleUpdate(float deltaTime)
{
    if (mSliderWidget && mSliderWidget->IsMouseCaptured())
    {
        Point pos = ScreenToLocal(gInputsManager.mCursorPosition);
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
    if ((mDecPosWidget == nullptr) && (childWidget->mName == "#min"))
    {
        mDecPosWidget = childWidget;
        doSubscribe = true;
    }
    if ((mIncPosWidget == nullptr) && (childWidget->mName == "#max"))
    {
        mIncPosWidget = childWidget;
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

    if (mDecPosWidget && (mDecPosWidget == childWidget))
    {
        mDecPosWidget = nullptr;
    }

    if (mIncPosWidget && (mIncPosWidget == childWidget))
    {
        mIncPosWidget = nullptr;
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
    if (mSliderWidget == nullptr)
        return;
    if (sender == mDecPosWidget)
    {
        mSliderWidget->SetPositionX(mSliderWidget->GetPosition().x - 10);
    }

    if (sender == mIncPosWidget)
    {
        mSliderWidget->SetPositionX(mSliderWidget->GetPosition().x + 10);
    }

    if (sender == mSliderWidget)
    {
    }
}

void GuiSlider::HandleMouseUp(GuiWidget* sender, eMouseButton mbutton)
{
    if (sender == mDecPosWidget)
    {

    }

    if (sender == mIncPosWidget)
    {

    }

    if (sender == mSliderWidget)
    {

    }
}