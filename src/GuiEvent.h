#pragma once

#include "GuiDefs.h"
#include "GuiWidget.h"

// gui event type
enum eGuiEvent
{
    eGuiEvent_Click,
    eGuiEvent_MouseEnter,
    eGuiEvent_MouseLeave,
    eGuiEvent_MouseDown,
    eGuiEvent_MouseUp,

    eGuiEvent_All, // not an event
};
decl_enum_strings(eGuiEvent);

// base events struct of gui widget
struct GuiEvent
{
public:
    GuiEvent(GuiWidget* eventSender, eGuiEvent eventType)
        : mEventSender(eventSender)
        , mEventId(eventType)
        , mMouseButton()
    {
    }
    GuiEvent(GuiWidget* eventSender, eGuiEvent eventType, eMouseButton mouseButton)
        : mEventSender(eventSender)
        , mEventId(eventType)
        , mMouseButton()
    {
    }

public:
    GuiWidgetHandle mEventSender; // sender widget handle

    eGuiEvent mEventId;// event identifier

    eMouseButton mMouseButton; // eGuiEvent_MouseButtonDown, eGuiEvent_MouseButtonUp
};

// widget events handler
class GuiEventsHandler
{
    friend class GuiManager;

public:
    GuiEventsHandler() = default;
    virtual ~GuiEventsHandler();

    void Subscribe(GuiWidget* eventSource, eGuiEvent eventId);
    void Unsubscribe(GuiWidget* eventSource, eGuiEvent eventId);
    void Unsubscribe(GuiWidget* eventSource);
    void UnsubscribeAll();

protected:
    // overridables
    virtual void HandleClickEvent(GuiWidget* sender) {}
    virtual void HandleMouseEnter(GuiWidget* sender) {}
    virtual void HandleMouseLeave(GuiWidget* sender) {}
    virtual void HandleMouseDown(GuiWidget* sender, eMouseButton mbutton) {}
    virtual void HandleMouseUp(GuiWidget* sender, eMouseButton mbutton) {}

private:
    bool ProcessEvent(GuiEvent* eventData);

private:
    struct Subscription
    {
        eGuiEvent mEventsId;
        GuiWidget* mEventsSource = nullptr;
    };
    std::vector<Subscription> mSubscriptions;
};