#include "stdafx.h"
#include "GameSessionAware.h"
#include "GameWorld.h"
#include "GameMain.h"
#include "GameSession.h"

GameWorld& GameSessionAware::GetGameWorld() const
{
    return GetGameSession().GetGameWorld();
}

RoomManager& GameSessionAware::GetRoomManager() const
{
    return GetGameWorld().GetRoomManager();
}

GameObjectManager& GameSessionAware::GetObjectManager() const
{
    return GetGameWorld().GetGameObjects();
}

CreatureManager& GameSessionAware::GetCreatureManager() const
{
    return GetGameWorld().GetCreatureManager();
}

GameEventBus& GameSessionAware::GetGameEventBus() const
{
    return gGame.GetEventsBus();
}

ScenarioVariables& GameSessionAware::GetScenarioVariables() const
{
    ScenarioDefinition& scenarioDefinition = GetScenarioDefinition();
    return scenarioDefinition.mVariables;
}

EconomyService& GameSessionAware::GetEconomyService() const
{
    return GetGameSession().GetEconomyService();
}

GameSession& GameSessionAware::GetGameSession() const
{
    return gGame.GetGameSession();
}

ScenarioDefinition& GameSessionAware::GetScenarioDefinition() const
{
    return GetGameSession().GetScenarioDefinition();
}

Scene& GameSessionAware::GetScene() const
{
    return GetGameWorld().GetScene();
}
