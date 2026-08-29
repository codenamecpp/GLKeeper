#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiWidget.h"
#include "SimpleTimer.h"

//////////////////////////////////////////////////////////////////////////

class UiScrollBar: public UiWidget
    , private UiEventListener // handle child events
{
protected:

    //////////////////////////////////////////////////////////////////////////
    enum eScrollMode
    {
        eScrollMode_None,
        eScrollMode_Slider,
        eScrollMode_ArrowUp,
        eScrollMode_ArrowDown,
    };
    //////////////////////////////////////////////////////////////////////////

public:
    UiScrollBar();

    void SetScrollRange(int scrollMin, int scrollMax);
    void SetScrollPosition(int scrollPosition);

    // Load widget properties from json
    void Deserialize(const JsonElement& jsonElement) override;

protected:
    UiScrollBar(const std::string& widgetClassName);
    UiScrollBar(const UiScrollBar& sourceWidget); // clone props
    ~UiScrollBar();

    void StartScroll(eScrollMode scrollMode);
    void FinishScroll();

    void ProcessArrowScrollStep();
    void ProcessScrollChanged(int prevScroll);

    void SetCurrScrollFromSliderPosition();
    void SetSliderPositionFromCurrScroll();
    void SetSliderPositionFromMouse(const Point2D& mouseScreenPosition);

    // override UiWidget
    UiScrollBar* CloneSelf() const override;
    void HandleChildAttached(UiWidget* widget) override;
    void HandleChildDetached(UiWidget* widget) override;
    void UpdateSelf(float deltaTime) override;

    // override UiEventListener
    void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc) override;

protected:
    eScrollMode mScrollMode = eScrollMode_None;
    UiWidget* mArrowUp {};
    UiWidget* mArrowDown {};
    UiWidget* mSlider {};
    Point2D mDragStartOffset {};

    int mScrollMin = 0;
    int mScrollMax = 2;
    int mScrollCurr = 0;

    SimpleTimer mArrowScrollTimer;
};

//////////////////////////////////////////////////////////////////////////