#include "pch.h"
#include "GuiScrollbar.h"
#include "GuiManager.h"
#include "InputsManager.h"
#include "GuiRenderer.h"
#include "Console.h"

// widget class factory
static GuiWidgetFactory<GuiScrollbar> _ScrollbarWidgetsFactory;

GuiWidgetMetaClass GuiScrollbar::MetaClass("scrollbar", &_ScrollbarWidgetsFactory, &GuiWidget::MetaClass);

GuiScrollbar::GuiScrollbar(): GuiScrollbar(&MetaClass)
{
}

GuiScrollbar::GuiScrollbar(GuiWidgetMetaClass* widgetClass)
    : GuiWidget(widgetClass)
{
}

GuiScrollbar::GuiScrollbar(GuiScrollbar* copyWidget)
    : GuiWidget(copyWidget)
{
}

void GuiScrollbar::SetMin(int minValue)
{
    mMin = minValue;
}

void GuiScrollbar::SetMax(int maxValue)
{
    mMax = maxValue;
}

void GuiScrollbar::SetPageSize(int pageSize)
{
    mPageSize = pageSize;
}

void GuiScrollbar::SetupControlWidget(GuiWidget* controlWidget)
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

void GuiScrollbar::HandleUpdate(float deltaTime)
{
    if (mSliderWidget && mSliderWidget->IsMouseCaptured())
    {
        Point pos = ScreenToLocal(gInputsManager.mCursorPosition);
        mSliderWidget->SetPositionX(pos.x);
    }
}

void GuiScrollbar::HandleRender(GuiRenderer& renderContext)
{
    Rectangle rc = GetLocalRect();
    renderContext.FillRect(rc, Color32_White);
}

bool GuiScrollbar::HasAttribute(eGuiWidgetAttribute attribute) const
{
    switch (attribute)
    {
        case eGuiWidgetAttribute_Interactive:
            return true;
    }
    return false;
}

void GuiScrollbar::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
}

void GuiScrollbar::HandleChildAttached(GuiWidget* childWidget)
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

void GuiScrollbar::HandleChildDetached(GuiWidget* childWidget)
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

GuiScrollbar* GuiScrollbar::ConstructClone()
{
    GuiScrollbar* selfClone = new GuiScrollbar(this);
    return selfClone;
}

void GuiScrollbar::HandleMouseDown(GuiWidget* sender, eMouseButton mbutton)
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

void GuiScrollbar::HandleMouseUp(GuiWidget* sender, eMouseButton mbutton)
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