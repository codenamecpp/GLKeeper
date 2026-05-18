#include "stdafx.h"
#include "GameEventBus.h"

void GameEventBus::ClearEvents()
{
    mEventQueue.clear();
    mEventQueueBuffer.clear();
}

void GameEventBus::Send_ReturnToFrontendRequest()
{
    GameEvent& ev = mEventQueue.emplace_back(eGameEvent_ReturnToFrontendRequest);
}

void GameEventBus::Send_StartScenarioRequest(const std::string& scenarioName)
{
    GameEvent& ev = mEventQueue.emplace_back(eGameEvent_StartScenarioRequest);
    ev.mStartScenarioRequest.mScenarioName = scenarioName;
}

void GameEventBus::Send_QuitGameRequest()
{
    GameEvent& ev = mEventQueue.emplace_back(eGameEvent_QuitGameRequest);
}

void GameEventBus::Send_ResourceAmountChanged(ePlayerID playerId, eGameResource resourceType)
{
    // discard if duplicate
    if (cxx::contains_if(mEventQueue, [playerId, resourceType](const GameEvent& gameEvent)
        {
            return gameEvent.IsEvent(eGameEvent_ResourceAmountChanged) &&
                (gameEvent.mResourceAmountChanged.mResourceType == resourceType) &&
                (gameEvent.mPlayerId == playerId);
        }))
    {
        return;
    }
    GameEvent& ev = mEventQueue.emplace_back(eGameEvent_ResourceAmountChanged);
    ev.mResourceAmountChanged.mResourceType = resourceType;
    ev.mPlayerId = playerId;
}

void GameEventBus::DispatchEvents()
{
    if (mEventQueue.empty()) return;

    mEventQueueBuffer.clear();
    
    std::swap(mEventQueueBuffer, mEventQueue);

    for (const GameEvent& ev: mEventQueueBuffer)
    {
        std::vector<GameEventListener*>& subscriptions = mSubscriptions[ev.mEventId];
        // using indexing instead of range-for for stability
        for (size_t i = 0; i < subscriptions.size(); ++i)
        {
            GameEventListener* listener = subscriptions[i];
            if (listener == nullptr)
                continue;

            listener->HandleGameEvent(ev);
        }
    }
}

void GameEventBus::Subscribe(eGameEvent eventID, GameEventListener* listener)
{
    if ((listener == nullptr) || 
        (eventID == eGameEvent_Null) || 
        (eventID >= eGameEvent_COUNT))
    {
        cxx_assert(false);
        return;
    }

    std::vector<GameEventListener*>& subscriptions = mSubscriptions[eventID];
    if (cxx::contains(subscriptions, listener))
        return;

    int reusePlaceIndex = cxx::get_first_index_if(subscriptions, [](GameEventListener* ptr) { return ptr == nullptr; });
    if (reusePlaceIndex != -1)
    {
        cxx_assert(subscriptions[reusePlaceIndex] == nullptr); // sanity check
        subscriptions[reusePlaceIndex] = listener;
    }
    else
    {
        subscriptions.push_back(listener);
    }
}

void GameEventBus::Unsubscribe(eGameEvent eventID, GameEventListener* listener)
{
    if ((listener == nullptr) || 
        (eventID == eGameEvent_Null) || 
        (eventID >= eGameEvent_COUNT))
    {
        cxx_assert(false);
        return;
    }

    std::vector<GameEventListener*>& subscriptions = mSubscriptions[eventID];
    
    int placeIndex = cxx::get_item_index(subscriptions, listener);
    if (placeIndex != -1)
    {
        cxx_assert(subscriptions[placeIndex] == listener); // sanity check
        subscriptions[placeIndex] = nullptr;
    }
}

void GameEventBus::Unsubscribe(GameEventListener* listener)
{
    cxx_assert(listener);
    if (listener == nullptr) return;

    for (std::vector<GameEventListener*>& subscriptions: mSubscriptions)
    {
        int placeIndex = cxx::get_item_index(subscriptions, listener);
        if (placeIndex != -1)
        {
            cxx_assert(subscriptions[placeIndex] == listener); // sanity check
            subscriptions[placeIndex] = nullptr;
        }
    }
}
