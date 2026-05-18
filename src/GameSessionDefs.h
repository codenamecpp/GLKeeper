#pragma once

//////////////////////////////////////////////////////////////////////////

class GameSession;
class GameSessionController;
class GameplayController;
class FrontendController;

//////////////////////////////////////////////////////////////////////////

enum eGameSessionType
{
    eGameSession_None,
    eGameSession_Frontend, // main menu
    eGameSession_Campaign,
    eGameSession_Skirmish,
    eGameSession_MyPetDungeon,
    eGameSession_Special, // secret level
    eGameSession_Level, // standalone
};

//////////////////////////////////////////////////////////////////////////

enum eGameSessionState
{
    eGameSessionState_None,
    eGameSessionState_Loaded,
    eGameSessionState_Active,
    eGameSessionState_Finished,
};

//////////////////////////////////////////////////////////////////////////

// Defines startup parameters for new game session
struct GameSessionStartupParams
{
public:
    GameSessionStartupParams()
        : mSessionType(eGameSession_None)
        , mScenarioName()
    {
    }

    // Reset startup parameters to defaults
    inline void Clear()
    {
        mSessionType = eGameSession_None;
        mScenarioName.clear();
    }

public:
    eGameSessionType mSessionType;

    std::string mScenarioName;
};

//////////////////////////////////////////////////////////////////////////