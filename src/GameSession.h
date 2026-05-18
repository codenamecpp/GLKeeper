#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameSessionDefs.h"
#include "Player.h"
#include "EconomyService.h"
#include "GameWorld.h"
#include "GameSessionController.h"

//////////////////////////////////////////////////////////////////////////

class GameSession: public cxx::noncopyable
{
public:

    bool Preload(GameLoadingAware& loadingContext, const GameSessionStartupParams& startupParams);
    void StartSession();
    void ShutdownSession();

    inline eGameSessionState GetSessionState() const { return mSessionState; }

    void UpdateFrame(float deltaTime);
    void UpdateLogic(float stepDeltaTime);
    void UpdatePhysics(float stepDeltaTime);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);

    // accessing players
    inline ePlayerID GetLocalPlayerID() const { return ePlayerID_Keeper1; }
    inline Player& GetLocalPlayer()
    {
        const ePlayerID pid = GetLocalPlayerID();
        return mPlayers[pid];
    }
    inline cxx::span<Player> GetPlayers() { return mPlayers; }
    inline Player& GetPlayer(ePlayerID pid)
    {
        return (pid < ePlayerID_COUNT) ? mPlayers[pid] : mPlayers[ePlayerID_Null];
    }

    // accessing world
    inline GameWorld& GetGameWorld() { return mGameWorld; }

    // accessing enonomy service
    inline EconomyService& GetEconomyService() { return mEconomyService; }

    // accessing scenario definitions
    inline ScenarioDefinition& GetScenarioDefinition() { return mScenarioData; } 

private:
    void ConfigurePlayers(const ScenarioDefinition& scenarioDefinition);
    void ClearScenarioData();

private:
    GameSessionStartupParams mSessionStartupParams;

    ScenarioDefinition mScenarioData;

    EconomyService mEconomyService;

    eGameSessionState mSessionState = eGameSessionState_None;
    std::unique_ptr<GameSessionController> mSessionController;

    Player mPlayers[ePlayerID_COUNT];
    GameWorld mGameWorld;
};

//////////////////////////////////////////////////////////////////////////