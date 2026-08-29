#include "stdafx.h"
#include "UiScrollBar.h"

UiScrollBar::UiScrollBar(): UiScrollBar("scrollbar")
{
}

UiScrollBar::UiScrollBar(const std::string& widgetClassName)
    : UiWidget(widgetClassName)
{
}

UiScrollBar::UiScrollBar(const UiScrollBar& sourceWidget)
    : UiWidget(sourceWidget)
    , mScrollMin(sourceWidget.mScrollMin)
    , mScrollMax(sourceWidget.mScrollMax)
    , mScrollCurr(sourceWidget.mScrollCurr)
{
}

void UiScrollBar::SetScrollRange(int scrollMin, int scrollMax)
{
    mScrollMin = std::min(scrollMin, scrollMax);
    mScrollMax = std::max(scrollMin, scrollMax);
    mScrollCurr = mScrollMin;
    SetSliderPositionFromCurrScroll();
}

void UiScrollBar::SetScrollPosition(int scrollPosition)
{
    const int prevPosition = mScrollCurr;
    mScrollCurr = std::clamp(scrollPosition, mScrollMin, mScrollMax);
    if (mScrollCurr != prevPosition)
    {
        SetSliderPositionFromCurrScroll();
    }
}

UiScrollBar::~UiScrollBar()
{
}

void UiScrollBar::StartScroll(eScrollMode scrollMode)
{
    cxx_assert(mScrollMode == eScrollMode_None);
    mScrollMode = scrollMode;
    cxx_assert(mScrollMode != eScrollMode_None);

    if ((mScrollMode == eScrollMode_ArrowDown) || 
        (mScrollMode == eScrollMode_ArrowUp))
    {
        mArrowScrollTimer.Start(0.5f);
        ProcessArrowScrollStep();
    }
}

void UiScrollBar::FinishScroll()
{
    mArrowScrollTimer.Stop();
    mScrollMode = eScrollMode_None;
    mDragStartOffset = {};
    SetCurrScrollFromSliderPosition();
    SetSliderPositionFromCurrScroll();
}

void UiScrollBar::ProcessArrowScrollStep()
{
    int delta = 0;
    switch (mScrollMode)
    {
        case eScrollMode_ArrowUp: delta = -1; break;
        case eScrollMode_ArrowDown: delta = 1; break;
    }
    cxx_assert(delta);
    const int prevScroll = mScrollCurr;
    mScrollCurr = std::clamp(mScrollCurr + delta, mScrollMin, mScrollMax);
    SetSliderPositionFromCurrScroll();
    ProcessScrollChanged(prevScroll);
}

void UiScrollBar::ProcessScrollChanged(int prevScroll)
{
    if (prevScroll == mScrollCurr)
        return;

    // notify
    const UiEvent_OnChangeScroll eventDesc {Point2D{0, mScrollCurr}};
    mEventListeners.IterateListeners([this, &eventDesc](UiEventListener* listener)
        {
            listener->HandleUiEvent(this, eventDesc);
        });
}

void UiScrollBar::SetSliderPositionFromCurrScroll()
{
    UiWidget* scrollAreaWidget = mSlider ? mSlider->GetParent() : nullptr;
    if (scrollAreaWidget == nullptr)
    {
        return;
    }

    float progress = 0.0f;
    {
        const int scrollCurr = std::clamp(mScrollCurr, mScrollMin, mScrollMax);
        const int scrollRange = (mScrollMax - mScrollMin);
        const int scrollDist = (mScrollCurr - mScrollMin);
        if ((scrollRange > 0) && (scrollDist > 0))
        {
            progress = (scrollDist * 1.0f) / (scrollRange * 1.0f);
        }
    }

    const Point2D scrollAreaSize = scrollAreaWidget->GetSize();
    const Point2D sliderSize = mSlider->GetSize();

    const int trackLength = (scrollAreaSize.y - sliderSize.y);
    if (trackLength < 1)
    {
        return;
    }

    const int sliderVertPosition = static_cast<int>(std::round(trackLength * progress));

    mSlider->SetPosition(Point2D(0, sliderVertPosition));
}

void UiScrollBar::SetCurrScrollFromSliderPosition()
{
    UiWidget* scrollAreaWidget = mSlider ? mSlider->GetParent() : nullptr;
    if (scrollAreaWidget == nullptr)
    {
        return;
    }

    const Point2D scrollAreaSize = scrollAreaWidget->GetSize();
    const Point2D sliderSize = mSlider->GetSize();
    const Point2D sliderPosition = mSlider->GetPosition();

    const int trackLength = (scrollAreaSize.y - sliderSize.y);
    if (trackLength < 1)
    {
        return;
    }
    const int sliderVertPosition = glm::clamp(sliderPosition.y, 0, trackLength);

    float scrollProgress = (sliderVertPosition * 1.0f / trackLength * 1.0f);
    mScrollCurr = static_cast<int>(std::round(glm::mix(mScrollMin * 1.0f, mScrollMax * 1.0f, scrollProgress)));
}

void UiScrollBar::SetSliderPositionFromMouse(const Point2D& mouseScreenPosition)
{
    UiWidget* scrollAreaWidget = mSlider ? mSlider->GetParent() : nullptr;
    if (scrollAreaWidget == nullptr)
    {
        return;
    }
    const Point2D scrollAreaSize = scrollAreaWidget->GetSize();
    const Point2D sliderSize = mSlider->GetSize();
    const Point2D targetPosition = scrollAreaWidget->ScreenToLocal(mouseScreenPosition) - mDragStartOffset;
    const Point2D newPosition {
        0, 
        std::clamp(targetPosition.y, 0, scrollAreaSize.y - sliderSize.y)
    };
    mSlider->SetPosition(newPosition);
}

UiScrollBar* UiScrollBar::CloneSelf() const
{
    UiScrollBar* clone = new UiScrollBar(*this);
    return clone;
}

void UiScrollBar::HandleChildAttached(UiWidget* widget)
{
    if (mArrowUp == nullptr)
    {
        mArrowUp = FindChildWithName("arrow_up");
        if (mArrowUp)
        {
            mArrowUp->Subscribe(this);
        }
    }

    if (mArrowDown == nullptr)
    {
        mArrowDown = FindChildWithName("arrow_down");
        if (mArrowDown)
        {
            mArrowDown->Subscribe(this);
        }
    }

    if (mSlider == nullptr)
    {
        mSlider = FindChildWithName("slider");
        if (mSlider)
        {
            mSlider->Subscribe(this);
        }
        SetSliderPositionFromCurrScroll();
    }
}

void UiScrollBar::HandleChildDetached(UiWidget* widget)
{
    if (widget == mArrowUp)
    {
        FinishScroll();
        mArrowUp->Unsubscribe(this);
        mArrowUp = nullptr;
    }

    if (widget == mArrowDown)
    {
        FinishScroll();
        mArrowDown->Unsubscribe(this);
        mArrowDown = nullptr;
    }

    if (widget == mSlider)
    {
        FinishScroll();
        mSlider->Unsubscribe(this);
        mSlider = nullptr;
    }
}

void UiScrollBar::UpdateSelf(float deltaTime)
{
    if (mArrowScrollTimer.IsOngoing() &&
        mArrowScrollTimer.TickAndCheckExpire(deltaTime))
    {
        ProcessArrowScrollStep();
        mArrowScrollTimer.Start(0.05f);
    }
}

void UiScrollBar::HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
{
    switch (mScrollMode)
    {
        case eScrollMode_None:
        {
            if ((sender == mSlider) && eventDesc.IsEvent(UiEventId_OnPress))
            {
                if (UiWidget* sliderBoundsWidget = mSlider->GetParent())
                {
                    mDragStartOffset = mSlider->ScreenToLocal(eventDesc.mMouseScreenPosition);
                    mDragStartOffset.x = 0;
                    StartScroll(eScrollMode_Slider);
                }
                break;
            }

            if ((sender == mArrowUp) && eventDesc.IsEvent(UiEventId_OnPress))
            {
                StartScroll(eScrollMode_ArrowUp);
                break;
            }

            if ((sender == mArrowDown) && eventDesc.IsEvent(UiEventId_OnPress))
            {
                StartScroll(eScrollMode_ArrowDown);
                break;
            }
        }
        break;

        case eScrollMode_Slider:
        {
            if ((sender == mSlider) && eventDesc.IsEvent(UiEventId_OnRelease))
            {
                FinishScroll();
                break;
            }

            if ((sender == mSlider) && eventDesc.IsEvent(UiEventId_OnMouseMove))
            {
                const int prevScroll = mScrollCurr;
                SetSliderPositionFromMouse(eventDesc.mMouseScreenPosition);
                SetCurrScrollFromSliderPosition();
                ProcessScrollChanged(prevScroll);
                return;
            }
        }
        break;

        case eScrollMode_ArrowUp:
        {
            if ((sender == mArrowUp) && eventDesc.IsEvent(UiEventId_OnRelease))
            {
                FinishScroll();
                break;
            }
        }
        break;

        case eScrollMode_ArrowDown:
        {
            if ((sender == mArrowDown) && eventDesc.IsEvent(UiEventId_OnRelease))
            {
                FinishScroll();
                break;
            }
        }
        break;
    }

}

void UiScrollBar::Deserialize(const JsonElement& jsonElement)
{
    UiWidget::Deserialize(jsonElement);
}

