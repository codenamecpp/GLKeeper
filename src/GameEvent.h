#pragma once

//////////////////////////////////////////////////////////////////////////

enum eGameEvent
{
    eGameEvent_Null,

    // gamestate change
    eGameEvent_ReturnToFrontendRequest,
    eGameEvent_StartScenarioRequest,
    eGameEvent_QuitGameRequest,

    // gameplay
    eGameEvent_ResourceAmountChanged,

    eGameEvent_COUNT
};

//////////////////////////////////////////////////////////////////////////

struct GameEvent
{
public:
    GameEvent(eGameEvent eventId) 
        : mEventId(eventId)
    {}

    inline bool IsEvent(eGameEvent eventId) const { return (mEventId == eventId); }

public:
    eGameEvent mEventId;

    struct
    {
        std::string mScenarioName;
    } 
    mStartScenarioRequest;

    struct 
    {
        eGameResource mResourceType;
    }
    mResourceAmountChanged;

    // common
    ePlayerID mPlayerId = ePlayerID_Null;
};

//////////////////////////////////////////////////////////////////////////

class GameEventListener
{
public:
    virtual ~GameEventListener()
    {}

    virtual void HandleGameEvent(const GameEvent& eventData) = 0;
};

//////////////////////////////////////////////////////////////////////////