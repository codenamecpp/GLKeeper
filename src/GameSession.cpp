#include "stdafx.h"
#include "GameSession.h"
#include "GameMain.h"
#include "FrontendController.h"
#include "GameplayController.h"
#include "DK2ScenarioReader.h"
#include "GameWorld.h"
#include "EconomyService.h"

//////////////////////////////////////////////////////////////////////////

GameSession gGameSession;

//////////////////////////////////////////////////////////////////////////

bool GameSession::Preload(GameLoadingAware& loadingContext, const GameSessionStartupParams& startupParams)
{
    cxx_assert(startupParams.mSessionType != eGameSession_None);

    ShutdownSession();

    loadingContext.UpdateLoadingProgress(0.0f);

    std::string levelFile;
    if (!gFiles.LocateMapData(startupParams.mScenarioName, levelFile))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot locate level '%s'", startupParams.mScenarioName.c_str());
        return false;
    }

    // load level database
    DK2ScenarioReader scenarioLoader;
    if (!scenarioLoader.ReadScenarioData(levelFile, mScenarioData))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load scenario '%s'", startupParams.mScenarioName.c_str());
        return false;
    }

    mSessionStartupParams = startupParams;

    ConfigurePlayers(mScenarioData);

    bool isSuccess = gGameWorld.LoadScenario(mScenarioData, loadingContext);
    if (isSuccess)
    {
        mSessionState = eGameSessionState_Loaded;
        
        // create session controller
        switch (startupParams.mSessionType)
        {
            case eGameSession_None: break;
            case eGameSession_Frontend:
                mSessionController = std::make_unique<FrontendController>();
            break;
            default:
                mSessionController = std::make_unique<GameplayController>();
            break;
        }
    }
    else
    {
        gConsole.LogMessage(eLogLevel_Error, "Cannot initialize scenario '%s'", mSessionStartupParams.mScenarioName.c_str());
    }

    gEconomyService.EnterWorld();

    if (isSuccess && mSessionController)
    {
        mSessionController->OnSessionLoaded();
    }

    loadingContext.UpdateLoadingProgress(1.0f);

    return isSuccess;
}

void GameSession::StartSession()
{
    if (mSessionState != eGameSessionState_Loaded)
    {
        cxx_assert(false);
        return;
    }

    mSessionState = eGameSessionState_Active;
    if (mSessionController)
    {
        mSessionController->OnSessionStart();
    }
}

void GameSession::ShutdownSession()
{
    if (mSessionState == eGameSessionState_None) 
        return;

    mSessionState = eGameSessionState_None;
    gEconomyService.ClearWorld();
    if (mSessionController)
    {
        mSessionController->OnSessionShutdown();
    }
    gGameWorld.ClearWorld();
    mSessionStartupParams.Clear();
    mSessionController.reset();
    ClearScenarioData();
}

void GameSession::UpdateFrame(float deltaTime)
{
    if (mSessionController)
    {
        mSessionController->UpdateFrame(deltaTime);
    }

    gGameWorld.UpdateFrame(deltaTime);
    gEconomyService.UpdateFrame(deltaTime);
}

void GameSession::UpdateLogic(float stepDeltaTime)
{
    gGameWorld.UpdateLogic(stepDeltaTime);
    gEconomyService.UpdateLogic(stepDeltaTime);
}

void GameSession::UpdatePhysics(float stepDeltaTime)
{
    gGameWorld.UpdatePhysics(stepDeltaTime);
}

void GameSession::InputEvent(KeyInputEvent& inputEvent)
{
    if (mSessionController)
    {
        mSessionController->InputEvent(inputEvent);
    }
}

void GameSession::InputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mSessionController)
    {
        mSessionController->InputEvent(inputEvent);
    }
}

void GameSession::InputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mSessionController)
    {
        mSessionController->InputEvent(inputEvent);
    }
}

void GameSession::InputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mSessionController)
    {
        mSessionController->InputEvent(inputEvent);
    }
}

void GameSession::ConfigurePlayers(const ScenarioDefinition& scenarioDefinition)
{
    for (int iplayer = 0; iplayer < ePlayerID_COUNT; ++iplayer)
    {
        ePlayerID playerId;
        if (!cxx::parse_enum_int(iplayer, playerId))
        {
            cxx_assert(false);
            continue;
        }

        PlayerDefinition* playerDefinition = scenarioDefinition.GetPlayerDefinition(playerId);
        if (playerDefinition == nullptr)
        {
            mPlayers[iplayer].Configure(playerId, ePlayerType_Null, "");
            continue;
        }

        Player& player = mPlayers[iplayer];

        player.Configure(playerId, playerDefinition->mPlayerType, playerDefinition->mPlayerName);
        player.SetStartCameraTilePosition({playerDefinition->mStartCameraX, playerDefinition->mStartCameraY});
        player.SetStartingResourceAmount(eGameResource_Gold, playerDefinition->mInitialGold);
        player.SetStartingResourceAmount(eGameResource_Mana, playerDefinition->mInitialMana);
    }
}

void GameSession::ClearScenarioData()
{
    mScenarioData = ScenarioDefinition{};
}
