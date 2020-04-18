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
    if (eventSource == nullptr || eventId == 0)
    {
        debug_assert(false);
        return;
    }

    bool registerEventsHandler = mSubscriptions.empty();

    eGuiEvent& currentEventsMask = mSubscriptions[eventSource];
    currentEventsMask = (currentEventsMask | eventId);

    if (registerEventsHandler)
    {
        gGuiManager.RegisterEventsHandler(this);
    }
}

void GuiEventsHandler::Unsubscribe(GuiWidget* eventSource, eGuiEvent eventId)
{
    debug_assert(eventSource);

    auto map_iterator = mSubscriptions.find(eventSource);
    if (map_iterator == mSubscriptions.end())
        return;

    eGuiEvent& currentEventsMask = map_iterator->second;
    currentEventsMask = currentEventsMask & ~(eventId);
    if (currentEventsMask == 0)
    {
        mSubscriptions.erase(map_iterator);
    }

    if (mSubscriptions.empty())
    {
        gGuiManager.UnregisterEventsHandler(this);
    }
}

void GuiEventsHandler::Unsubscribe(GuiWidget* eventSource)
{
    Unsubscribe(eventSource, GuiEvent_All);
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

    auto map_iterator = mSubscriptions.find(eventData->mEventSender);
    if (map_iterator == mSubscriptions.end())
        return;

    if ((map_iterator->second & eventData->mEventId) == 0)
        return;

    switch (eventData->mEventId)
    {
        case eGuiEvent_Click: 
            HandleClickEvent(eventData->mEventSender); 
        break;
        case eGuiEvent_MouseEnter:
            HandleMouseEnter(eventData->mEventSender);
        break;
        case eGuiEvent_MouseLeave:
            HandleMouseLeave(eventData->mEventSender);
        break;
        case eGuiEvent_MouseDown:
            HandleMouseDown(eventData->mEventSender, eventData->mMouseButton);
        break;
        case eGuiEvent_MouseUp:
            HandleMouseUp(eventData->mEventSender, eventData->mMouseButton);
        break;
    }
}