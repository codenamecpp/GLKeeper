#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameEvent.h"

//////////////////////////////////////////////////////////////////////////

class GameEventBus: public cxx::noncopyable
{
public:
    GameEventBus() = default;

    //////////////////////////////////////////////////////////////////////////
    // message events
    //////////////////////////////////////////////////////////////////////////

    void Send_ReturnToFrontendRequest();
    void Send_StartScenarioRequest(const std::string& scenarioName);
    void Send_QuitGameRequest();

    void Send_ResourceAmountChanged(ePlayerID playerId, eGameResource resourceType);

    //////////////////////////////////////////////////////////////////////////
    // management
    //////////////////////////////////////////////////////////////////////////

    // process queued events
    void DispatchEvents();

    // discard all queued events without sending
    void ClearEvents();

    // subscriptions
    void Subscribe(eGameEvent eventID, GameEventListener* listener);
    void Unsubscribe(eGameEvent eventID, GameEventListener* listener);
    void Unsubscribe(GameEventListener* listener);

private:
    std::vector<GameEventListener*> mSubscriptions[eGameEvent_COUNT];
    std::vector<GameEvent> mEventQueue;
    std::vector<GameEvent> mEventQueueBuffer;
};

//////////////////////////////////////////////////////////////////////////