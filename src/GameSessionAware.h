#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameSessionDefs.h"
#include "GameWorldDefs.h"
#include "GameDefs.h"
#include "RoomDefs.h"
#include "GameObjectDefs.h"
#include "ScenarioDefs.h"
#include "SceneDefs.h"
#include "CreatureDefs.h"

//////////////////////////////////////////////////////////////////////////

class GameSessionAware: public cxx::noncopyable
{
protected:
    GameSessionAware() = default;

    //////////////////////////////////////////////////////////////////////////
    // provides shortcuts
    //////////////////////////////////////////////////////////////////////////

    GameSession&        GetGameSession() const;
    ScenarioDefinition& GetScenarioDefinition() const;
    Scene&              GetScene() const;
    GameWorld&          GetGameWorld() const;
    RoomManager&        GetRoomManager() const;
    GameObjectManager&  GetObjectManager() const;
    CreatureManager&    GetCreatureManager() const;
    GameEventBus&       GetGameEventBus() const;
    ScenarioVariables&  GetScenarioVariables() const;
    EconomyService&     GetEconomyService() const;
};

//////////////////////////////////////////////////////////////////////////