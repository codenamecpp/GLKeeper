#pragma once

#include "GuiDefs.h"

// gui event type
enum eGuiEvent
{
    eGuiEvent_MouseClick,
};
decl_enum_strings(eGuiEvent);

// gui widget events struct
struct GuiEvent
{
public:
    // allocate new event
    static GuiEvent* GetEvent(GuiWidget* eventSender, eGuiEvent eventType)
    {
        GuiEvent* ev = GetEventsPool().create(eventSender, eventType);
        return ev;
    }

    // free event
    void FreeEvent()
    {
        GetEventsPool().destroy(this);
    }

public:
    GuiWidgetHandle mEventSender;
    
    eGuiEvent mEventType;// event identifier

    // event specific properties
    union
    {
        struct // eGuiEvent_MouseClick
        {

        } mMouseClick;
    };

private:
    // implementation details

    friend class cxx::details::object_pool_node<GuiEvent>;

    GuiEvent(GuiWidget* eventSender, eGuiEvent eventType)
        : mEventSender(eventSender)
        , mEventType(eventType)
    {
    }

    ~GuiEvent()
    {
    }

    using GuiEventsPool = cxx::object_pool<GuiEvent>;
    static GuiEventsPool& GetEventsPool()
    {
        static GuiEventsPool sEventsPool;
        return sEventsPool;
    }
};