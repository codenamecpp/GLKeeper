#pragma once

#include "GuiDefs.h"
#include "GuiWidget.h"

// gui events
extern const cxx::unique_string GuiEventId_Click;
extern const cxx::unique_string GuiEventId_MouseEnter;
extern const cxx::unique_string GuiEventId_MouseLeave;
extern const cxx::unique_string GuiEventId_MouseDown;
extern const cxx::unique_string GuiEventId_MouseUp;

// base events struct of gui widget
struct GuiEvent
{
public:
    cxx::unique_string mEventId;// event identifier
    GuiWidgetHandle mEventSender; // sender widget handle

    // event args
    eMouseButton mMouseButton; // eGuiEvent_MouseButtonDown, eGuiEvent_MouseButtonUp
    Point mMouseScreenPosition; // eGuiEvent_MouseEnter, eGuiEvent_MouseLeave, eGuiEvent_MouseDown, eGuiEvent_MouseUp, eGuiEvent_Click

public:

    // event construction helpers

    static GuiEvent ForCustomEvent(GuiWidget* eventSender, cxx::unique_string eventId)
    {
        GuiEvent ev (eventSender, eventId);
        return ev;
    }

    static GuiEvent ForClick(GuiWidget* eventSender, const Point& screenPosition)
    {
        GuiEvent ev (eventSender, GuiEventId_Click);
        ev.mMouseScreenPosition = screenPosition;
        return ev;
    }

    static GuiEvent ForMouseEnter(GuiWidget* eventSender, const Point& screenPosition)
    {
        GuiEvent ev (eventSender, GuiEventId_MouseEnter);
        ev.mMouseScreenPosition = screenPosition;
        return ev;
    }

    static GuiEvent ForMouseLeave(GuiWidget* eventSender, const Point& screenPosition)
    {
        GuiEvent ev (eventSender, GuiEventId_MouseLeave);
        ev.mMouseScreenPosition = screenPosition;
        return ev;
    }

    static GuiEvent ForMouseDown(GuiWidget* eventSender, eMouseButton mouseButton, const Point& screenPosition)
    {
        GuiEvent ev (eventSender, GuiEventId_MouseDown);
        ev.mMouseButton = mouseButton;
        ev.mMouseScreenPosition = screenPosition;
        return ev;
    }

    static GuiEvent ForMouseUp(GuiWidget* eventSender, eMouseButton mouseButton, const Point& screenPosition)
    {
        GuiEvent ev (eventSender, GuiEventId_MouseUp);
        ev.mMouseButton = mouseButton;
        ev.mMouseScreenPosition = screenPosition;
        return ev;
    }

private:
    // use helpers instead of direct construction
    GuiEvent(GuiWidget* eventSender, cxx::unique_string eventId)
        : mEventSender(eventSender)
        , mEventId(eventId)
    {
    }
};

// widget events handler
class GuiEventsHandler
{
    friend class GuiManager;

public:
    GuiEventsHandler() = default;
    virtual ~GuiEventsHandler();

    // @param eventSource cannot be null
    void Subscribe(cxx::unique_string eventId, cxx::unique_string eventSource);
    void Subscribe(cxx::unique_string eventId, GuiWidget* eventSource);

    void Unsubscribe(cxx::unique_string eventId, cxx::unique_string eventSource);
    void Unsubscribe(cxx::unique_string eventId);
    void Unsubscribe(cxx::unique_string eventId, GuiWidget* eventSource);
    void UnsubscribeEventsSource(cxx::unique_string eventSource);
    void UnsubscribeEventsSource(GuiWidget* eventSource);
    void UnsubscribeAll();

    bool HasSubscriptions() const;

protected:
    // overridables
    virtual void HandleClick(GuiWidget* sender) {}
    virtual void HandleMouseEnter(GuiWidget* sender) {}
    virtual void HandleMouseLeave(GuiWidget* sender) {}
    virtual void HandleMouseDown(GuiWidget* sender, eMouseButton mbutton) {}
    virtual void HandleMouseUp(GuiWidget* sender, eMouseButton mbutton) {}
    virtual void HandleEvent(GuiWidget* sender, cxx::unique_string eventId) {}

private:
    void ProcessEvent(GuiEvent* eventData);
    void RemoveEventsWithoutSubscribers();

private:
    using NameSubscriptions = std::vector<cxx::unique_string>; // widget names
    using PtrsSubscriptions = std::vector<GuiWidget*>; // widget pointers
    struct Subscriptions
    {
        NameSubscriptions mNames;
        PtrsSubscriptions mPointers;
    };
    std::map<cxx::unique_string, Subscriptions> mSubscriptions;
};