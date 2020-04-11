#pragma once

#include "GuiDefs.h"
#include "GuiWidget.h"

// gui event type
enum eGuiEvent
{
    eGuiEvent_Click,
    eGuiEvent_MouseEnter,
    eGuiEvent_MouseLeave,
};
decl_enum_strings(eGuiEvent);

// base events struct of gui widget
struct GuiEvent
{
public:
    GuiEvent(GuiWidget* eventSender, eGuiEvent eventType)
        : mEventSender(eventSender)
        , mEventId(eventType)
    {
    }

public:
    GuiWidgetHandle mEventSender; // sender widget handle

    eGuiEvent mEventId;// event identifier
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
    virtual void HandleClickEvent(GuiWidget* sender, GuiEvent* eventData) {}
    virtual void HandleMouseEnter(GuiWidget* sender, GuiEvent* eventData) {}
    virtual void HandleMouseLeave(GuiWidget* sender, GuiEvent* eventData) {}

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