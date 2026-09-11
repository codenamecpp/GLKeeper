#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameSessionDefs.h"
#include "Player.h"
#include "GameSessionController.h"

//////////////////////////////////////////////////////////////////////////

class GameSession final: public cxx::noncopyable
{
public:
    bool Preload(GameLoadingAware& loadingContext, const GameSessionStartupParams& startupParams);
    void StartSession();
    void ShutdownSession();

    inline eGameSessionState GetSessionState() const { return mSessionState; }
    inline bool IsInState(eGameSessionState sessionState) const
    {
        return mSessionState == sessionState;
    }

    void UpdateFrame(float deltaTime);
    void UpdateLogic(float stepDeltaTime);
    void UpdatePhysics(float stepDeltaTime);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);

    // accessing players
    inline bool IsLocalPlayerId(ePlayerID playerId) const
    {
        return playerId == GetLocalPlayerId();
    }
    inline ePlayerID GetLocalPlayerId() const { return ePlayerID_Keeper1; }
    inline Player& GetLocalPlayer()
    {
        const ePlayerID pid = GetLocalPlayerId();
        return mPlayers[pid];
    }
    inline cxx::span<Player> GetPlayers() { return mPlayers; }
    inline Player& GetPlayer(ePlayerID pid)
    {
        return (pid < ePlayerID_COUNT) ? mPlayers[pid] : mPlayers[ePlayerID_Null];
    }

    // accessing scenario definitions
    inline ScenarioDefinition& GetScenarioDefinition() { return mScenarioData; }
    inline const ScenarioVariables& GetScenarioVariables() const
    {
        return mScenarioData.mVariables;
    }

private:
    void ConfigurePlayers(const ScenarioDefinition& scenarioDefinition);
    void ClearScenarioData();

private:
    GameSessionStartupParams mSessionStartupParams;

    ScenarioDefinition mScenarioData;

    eGameSessionState mSessionState = eGameSessionState_None;
    std::unique_ptr<GameSessionController> mSessionController;

    Player mPlayers[ePlayerID_COUNT];
};

//////////////////////////////////////////////////////////////////////////

extern GameSession gGameSession;

//////////////////////////////////////////////////////////////////////////