#pragma once

#include "UiDefs.h"

//////////////////////////////////////////////////////////////////////////

enum UiEventID
{
    UiEventID_None,
    UiEventID_Notification, // user defined event

    UiEventID_OnPress,
    UiEventID_OnClick,

    // More widget events add here

    UiEvent_COUNT // must be very last
};

//////////////////////////////////////////////////////////////////////////

// Base Ui Event Desc
struct UiEventDesc
{
    UiEventDesc(UiEventID eventID): mEventID(eventID)
    {}
    UiEventID mEventID;
};

//////////////////////////////////////////////////////////////////////////

struct UiEvent_OnPress: public UiEventDesc
{
    UiEvent_OnPress(int mouseIdx): UiEventDesc(UiEventID_OnPress)
        , mMouseIdx(mouseIdx)
    {}
    int mMouseIdx;
};

//////////////////////////////////////////////////////////////////////////

struct UiEvent_OnClick: public UiEventDesc
{
    UiEvent_OnClick(int mouseIdx): UiEventDesc(UiEventID_OnClick)
        , mMouseIdx(mouseIdx)
    {}
    int mMouseIdx;
};

//////////////////////////////////////////////////////////////////////////

struct UiEvent_Notification: public UiEventDesc
{
    UiEvent_Notification(int code, int intParam): UiEventDesc(UiEventID_Notification)
        , mCode(code)
        , mParam(intParam)
    {}
    int mCode;
    int mParam;
};

//////////////////////////////////////////////////////////////////////////

class UiEventListener
{
public:
    virtual ~UiEventListener() {}

    // Process event
    // @param sender: Event source
    // @param eventDesc: Event info
    virtual void HandleUiEvent(UiWidget* sender, const UiEventDesc* eventDesc)
    {}
};
