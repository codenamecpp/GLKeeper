#pragma once

#include "GuiDefs.h"
#include "GuiWidget.h"

// gui events
extern const cxx::unique_string GuiEventId_OnClick;
extern const cxx::unique_string GuiEventId_OnMouseEnter;
extern const cxx::unique_string GuiEventId_OnMouseLeave;
extern const cxx::unique_string GuiEventId_OnMouseDown;
extern const cxx::unique_string GuiEventId_OnMouseUp;
extern const cxx::unique_string GuiEventId_OnEnable;
extern const cxx::unique_string GuiEventId_OnDisable;
extern const cxx::unique_string GuiEventId_OnShow;
extern const cxx::unique_string GuiEventId_OnHide;

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
    GuiEvent(GuiWidget* eventSender, cxx::unique_string eventId)
        : mEventSender(eventSender)
        , mEventId(eventId)
    {
    }

    // event construction helpers

    static GuiEvent ClickEvent(GuiWidget* eventSender, const Point& screenPosition)
    {
        GuiEvent ev (eventSender, GuiEventId_OnClick);
        ev.mMouseScreenPosition = screenPosition;
        return ev;
    }

    static GuiEvent MouseEnterEvent(GuiWidget* eventSender, const Point& screenPosition)
    {
        GuiEvent ev (eventSender, GuiEventId_OnMouseEnter);
        ev.mMouseScreenPosition = screenPosition;
        return ev;
    }

    static GuiEvent MouseLeaveEvent(GuiWidget* eventSender, const Point& screenPosition)
    {
        GuiEvent ev (eventSender, GuiEventId_OnMouseLeave);
        ev.mMouseScreenPosition = screenPosition;
        return ev;
    }

    static GuiEvent MouseDownEvent(GuiWidget* eventSender, eMouseButton mouseButton, const Point& screenPosition)
    {
        GuiEvent ev (eventSender, GuiEventId_OnMouseDown);
        ev.mMouseButton = mouseButton;
        ev.mMouseScreenPosition = screenPosition;
        return ev;
    }

    static GuiEvent MouseUpEvent(GuiWidget* eventSender, eMouseButton mouseButton, const Point& screenPosition)
    {
        GuiEvent ev (eventSender, GuiEventId_OnMouseUp);
        ev.mMouseButton = mouseButton;
        ev.mMouseScreenPosition = screenPosition;
        return ev;
    }

    bool ResolveCondition(const cxx::unique_string& identifier, bool& isTrue) const;
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