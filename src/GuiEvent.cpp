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

    bool registerEventsHandler = mSubscriptions.empty();

    Subscriptions& eventSourcesList = mSubscriptions[eventId];
    if (!cxx::contains(eventSourcesList, eventSource))
    {
        eventSourcesList.push_back(eventSource);
    }

    if (registerEventsHandler)
    {
        gGuiManager.RegisterEventsHandler(this);
    }
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

    Subscriptions& eventSourcesList = map_iterator->second;
    cxx::erase_elements(eventSourcesList, eventSource);

    if (eventSourcesList.empty())
    {
        mSubscriptions.erase(map_iterator);
    }

    if (mSubscriptions.empty())
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
    if (map_iterator == mSubscriptions.end())
        return;

    mSubscriptions.erase(map_iterator);
    if (mSubscriptions.empty())
    {
        gGuiManager.UnregisterEventsHandler(this);
    }
}

void GuiEventsHandler::UnsubscribeAll()
{
    mSubscriptions.clear();
    gGuiManager.UnregisterEventsHandler(this);
}

void GuiEventsHandler::ProcessEvent(GuiEvent* eventData)
{
    debug_assert(eventData);

    if (eventData->mEventSender == nullptr)
        return;

    auto map_iterator = mSubscriptions.find(eventData->mEventId);
    if (map_iterator == mSubscriptions.end())
        return;

    if (!cxx::contains(map_iterator->second, eventData->mEventSender->mName))
        return;

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

    HandleEvent(eventData->mEventId, eventData->mEventSender);
}