#include "pch.h"
#include "GuiEvent.h"
#include "GuiWidget.h"
#include "GuiManager.h"

const cxx::unique_string GuiEventId_Click("click");
const cxx::unique_string GuiEventId_MouseEnter("mouse_enter");
const cxx::unique_string GuiEventId_MouseLeave("mouse_leave");
const cxx::unique_string GuiEventId_MouseDown("mouse_down");
const cxx::unique_string GuiEventId_MouseUp("mouse_up");

GuiEventsHandler::~GuiEventsHandler()
{
    UnsubscribeAll();
}

void GuiEventsHandler::Subscribe(cxx::unique_string eventId, cxx::unique_string eventSource)
{
    if (eventId.empty() || eventSource.empty())
    {
        debug_assert(false);
        return;
    }

    if (!HasSubscriptions())
    {
        gGuiManager.RegisterEventsHandler(this);
    }

    Subscriptions& subscriptions = mSubscriptions[eventId];
    cxx::push_back_if_unique(subscriptions.mNames, eventSource);
}

void GuiEventsHandler::Subscribe(cxx::unique_string eventId, GuiWidget* eventSource)
{
    if (eventId.empty() || eventSource == nullptr)
    {
        debug_assert(false);
        return;
    }

    if (!HasSubscriptions())
    {
        gGuiManager.RegisterEventsHandler(this);
    }

    Subscriptions& subscriptions = mSubscriptions[eventId];
    cxx::push_back_if_unique(subscriptions.mPointers, eventSource);
}

void GuiEventsHandler::Unsubscribe(cxx::unique_string eventId, cxx::unique_string eventSource)
{
    if (eventId.empty() || eventSource.empty())
    {
        debug_assert(false);
        return;
    }

    auto map_iterator = mSubscriptions.find(eventId);
    if (map_iterator == mSubscriptions.end())
        return;

    Subscriptions& subscriptions = map_iterator->second;
    cxx::erase_elements(subscriptions.mNames, eventSource);

    if (subscriptions.mNames.empty() && subscriptions.mPointers.empty())
    {
        mSubscriptions.erase(map_iterator);
    }

    if (!HasSubscriptions())
    {
        gGuiManager.UnregisterEventsHandler(this);
    }
}

void GuiEventsHandler::Unsubscribe(cxx::unique_string eventId, GuiWidget* eventSource)
{
    if (eventId.empty() || eventSource == nullptr)
    {
        debug_assert(false);
        return;
    }

    auto map_iterator = mSubscriptions.find(eventId);
    if (map_iterator == mSubscriptions.end())
        return;

    Subscriptions& subscriptions = map_iterator->second;
    cxx::erase_elements(subscriptions.mPointers, eventSource);

    if (subscriptions.mNames.empty() && subscriptions.mPointers.empty())
    {
        mSubscriptions.erase(map_iterator);
    }

    if (!HasSubscriptions())
    {
        gGuiManager.UnregisterEventsHandler(this);
    }
}

void GuiEventsHandler::Unsubscribe(cxx::unique_string eventId)
{
    if (eventId.empty())
    {
        debug_assert(false);
        return;
    }

    auto map_iterator = mSubscriptions.find(eventId);
    if (map_iterator != mSubscriptions.end())
    {
        mSubscriptions.erase(map_iterator);
    }

    if (!HasSubscriptions())
    {
        gGuiManager.UnregisterEventsHandler(this);
    }
}

void GuiEventsHandler::UnsubscribeAll()
{
    mSubscriptions.clear();
    gGuiManager.UnregisterEventsHandler(this);
}

bool GuiEventsHandler::HasSubscriptions() const
{
    return !mSubscriptions.empty();
}

void GuiEventsHandler::ProcessEvent(GuiEvent* eventData)
{
    debug_assert(eventData);

    if (eventData->mEventSender == nullptr)
        return;

    auto map_iterator = mSubscriptions.find(eventData->mEventId);
    if (map_iterator == mSubscriptions.end())
        return;

    // check by pointer and name
    const Subscriptions& subscriptions = map_iterator->second;
    if (!cxx::contains(subscriptions.mPointers, eventData->mEventSender) &&
        !cxx::contains(subscriptions.mNames, eventData->mEventSender->mName))
    {
        return;
    }

    if (eventData->mEventId == GuiEventId_Click)
    {
        HandleClick(eventData->mEventSender); 
        return;
    }

    if (eventData->mEventId == GuiEventId_MouseEnter)
    {
        HandleMouseEnter(eventData->mEventSender);
        return;
    }

    if (eventData->mEventId == GuiEventId_MouseLeave)
    {
        HandleMouseLeave(eventData->mEventSender);
        return;
    }

    if (eventData->mEventId == GuiEventId_MouseDown)
    {
        HandleMouseDown(eventData->mEventSender, eventData->mMouseButton);
        return;
    }

    if (eventData->mEventId == GuiEventId_MouseUp)
    {
        HandleMouseUp(eventData->mEventSender, eventData->mMouseButton);
        return;
    }

    HandleEvent(eventData->mEventSender, eventData->mEventId);
}