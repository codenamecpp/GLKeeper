#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiDefs.h"

//////////////////////////////////////////////////////////////////////////

enum UiEventId
{
    UiEventId_None,

    UiEventId_OnPress,
    UiEventId_OnRelease,
    UiEventId_OnClick,
    UiEventId_OnMouseMove,
    UiEventId_Wheel,
    UiEventId_ChangeScroll,

    UiEventId_COUNT // must be very last
};

//////////////////////////////////////////////////////////////////////////

struct UiEvent
{
public:
    UiEvent(UiEventId eventId): mEventId(eventId)
    {}
    inline bool IsEvent(UiEventId eventId) const { return mEventId == eventId; }
public:
    const UiEventId mEventId;

    // event-specific data

    int mMouseButton {};

    Point2D mDelta {};
    Point2D mMouseScreenPosition {};
    Point2D mScroll {};
};

struct UiEvent_OnPress: public UiEvent
{
public:
    UiEvent_OnPress(int mouseButton, const Point2D& cursorScreenPosition)
        : UiEvent(UiEventId_OnPress)
    {
        mMouseButton = mouseButton;
        mMouseScreenPosition = cursorScreenPosition;
    }
};

struct UiEvent_OnRelease: public UiEvent
{
public:
    UiEvent_OnRelease(int mouseButton, const Point2D& cursorScreenPosition)
        : UiEvent(UiEventId_OnRelease)
    {
        mMouseButton = mouseButton;
        mMouseScreenPosition = cursorScreenPosition;
    }
};

struct UiEvent_OnClick: public UiEvent
{
public:
    UiEvent_OnClick(int mouseButton)
        : UiEvent(UiEventId_OnClick)
    {
        mMouseButton = mouseButton;
    }
};

struct UiEvent_OnMouseMove: public UiEvent
{
public:
    UiEvent_OnMouseMove(const Point2D& delta, const Point2D& cursorScreenPosition)
        : UiEvent(UiEventId_OnMouseMove)
    {
        mDelta = delta;
        mMouseScreenPosition = cursorScreenPosition;
    }
};

struct UiEvent_OnWheel: public UiEvent
{
public:
    UiEvent_OnWheel(const Point2D& delta, const Point2D& cursorScreenPosition)
        : UiEvent(UiEventId_Wheel)
    {
        mDelta = delta;
        mMouseScreenPosition = cursorScreenPosition;
    }
};

struct UiEvent_OnChangeScroll: public UiEvent
{
public:
    UiEvent_OnChangeScroll(const Point2D& scrollPosition)
        : UiEvent(UiEventId_ChangeScroll)
    {
        mScroll = scrollPosition;
    }
};

//////////////////////////////////////////////////////////////////////////

class UiEventListener
{
public:
    virtual ~UiEventListener() {}

    // Process event
    // @param sender: Event source
    // @param eventDesc: Event info
    virtual void HandleUiEvent(UiWidget* sender, const UiEvent& eventDesc)
    {}
};

//////////////////////////////////////////////////////////////////////////