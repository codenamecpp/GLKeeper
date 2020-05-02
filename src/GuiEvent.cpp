#include "pch.h"
#include "GuiEvent.h"
#include "GuiWidget.h"
#include "GuiManager.h"

const cxx::unique_string GuiEventId_OnClick("on_click");
const cxx::unique_string GuiEventId_OnMouseEnter("on_mouse_enter");
const cxx::unique_string GuiEventId_OnMouseLeave("on_mouse_leave");
const cxx::unique_string GuiEventId_OnMouseDown("on_mouse_down");
const cxx::unique_string GuiEventId_OnMouseUp("on_mouse_up");
const cxx::unique_string GuiEventId_OnEnable("on_enable");
const cxx::unique_string GuiEventId_OnDisable("on_disable");
const cxx::unique_string GuiEventId_OnShow("on_show");
const cxx::unique_string GuiEventId_OnHide("on_hide");

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

void GuiEventsHandler::UnsubscribeEventsSource(cxx::unique_string eventSource)
{
    if (eventSource.empty())
    {
        debug_assert(false);
        return;
    }

    for (auto& curr_iterator: mSubscriptions)
    {
        cxx::erase_elements(curr_iterator.second.mNames, eventSource);
    }
    RemoveEventsWithoutSubscribers();
    if (!HasSubscriptions())
    {
        gGuiManager.UnregisterEventsHandler(this);
    }
}

void GuiEventsHandler::UnsubscribeEventsSource(GuiWidget* eventSource)
{
    if (eventSource == nullptr)
    {
        debug_assert(false);
        return;
    }
    for (auto& curr_iterator: mSubscriptions)
    {
        cxx::erase_elements(curr_iterator.second.mPointers, eventSource);
    }
    RemoveEventsWithoutSubscribers();
    if (!HasSubscriptions())
    {
        gGuiManager.UnregisterEventsHandler(this);
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
    if (map_iterator != mSubscriptions.end())
    {
        cxx::erase_elements(map_iterator->second.mNames, eventSource);
    }
    RemoveEventsWithoutSubscribers();
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
    if (map_iterator != mSubscriptions.end())
    {
        cxx::erase_elements(map_iterator->second.mPointers, eventSource);
    }
    RemoveEventsWithoutSubscribers();
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
        !cxx::contains(subscriptions.mNames, eventData->mEventSender->mId))
    {
        return;
    }

    if (eventData->mEventId == GuiEventId_OnClick)
    {
        HandleClick(eventData->mEventSender); 
        return;
    }

    if (eventData->mEventId == GuiEventId_OnMouseEnter)
    {
        HandleMouseEnter(eventData->mEventSender);
        return;
    }

    if (eventData->mEventId == GuiEventId_OnMouseLeave)
    {
        HandleMouseLeave(eventData->mEventSender);
        return;
    }

    if (eventData->mEventId == GuiEventId_OnMouseDown)
    {
        HandleMouseDown(eventData->mEventSender, eventData->mMouseButton);
        return;
    }

    if (eventData->mEventId == GuiEventId_OnMouseUp)
    {
        HandleMouseUp(eventData->mEventSender, eventData->mMouseButton);
        return;
    }

    HandleEvent(eventData->mEventSender, eventData->mEventId);
}

void GuiEventsHandler::RemoveEventsWithoutSubscribers()
{
    for (auto map_iteartor = mSubscriptions.begin(); 
        map_iteartor != mSubscriptions.end();)
    {
        if (map_iteartor->second.mNames.empty() && map_iteartor->second.mPointers.empty())
        {
            map_iteartor = mSubscriptions.erase(map_iteartor);
        }
        else
        {
            ++map_iteartor;
        }
    }
}
