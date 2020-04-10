#include "pch.h"
#include "GuiEvent.h"
#include "GuiWidget.h"
#include "GuiManager.h"

GuiEventsHandler::~GuiEventsHandler()
{
    UnsubscribeAll();
}

void GuiEventsHandler::Subscribe(GuiWidget* eventSource, eGuiEvent eventId)
{
    if (eventSource == nullptr)
    {
        debug_assert(false);
        return;
    }

    bool subsFound = cxx::contains_if(mSubscriptions,
        [eventSource, eventId](const Subscription& curr)
        {
            return curr.mEventsSource == eventSource && curr.mEventsId == eventId;
        });

    if (subsFound)
        return;

    Subscription subs;
        subs.mEventsSource = eventSource;
        subs.mEventsId = eventId;
    mSubscriptions.push_back(subs);

    gGuiManager.RegisterEventsHandler(this);
}

void GuiEventsHandler::Unsubscribe(GuiWidget* eventSource, eGuiEvent eventId)
{
    if (eventSource == nullptr)
    {
        debug_assert(false);
        return;
    }

    cxx::erase_elements_if(mSubscriptions, [eventSource, eventId](const Subscription& curr)
        {
            return curr.mEventsSource == eventSource && curr.mEventsId == eventId;
        });

    if (mSubscriptions.empty())
    {
        gGuiManager.UnregisterEventsHandler(this);
    }
}

void GuiEventsHandler::Unsubscribe(GuiWidget* eventSource)
{
    if (eventSource == nullptr)
    {
        debug_assert(false);
        return;
    }

    cxx::erase_elements_if(mSubscriptions, [eventSource](const Subscription& curr)
        {
            return curr.mEventsSource == eventSource;
        });

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

void GuiEventsHandler::HandleEvent(GuiWidget* sender, GuiEvent* eventData)
{
    // do nothing
}

bool GuiEventsHandler::ProcessEvent(GuiEvent* eventData)
{
    debug_assert(eventData);

    if (eventData->mEventSender == nullptr)
        return false;

    for (const Subscription& curr: mSubscriptions)
    {
        if (curr.mEventsSource == eventData->mEventSender && curr.mEventsId == eventData->mEventId)
        {
            HandleEvent(curr.mEventsSource, eventData);
            return true;
        }
    }
    return false;
}