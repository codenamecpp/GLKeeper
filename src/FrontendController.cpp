#include "stdafx.h"
#include "FrontendController.h"
#include "GameWorld.h"
#include "GameMain.h"
#include "MapUtils.h"
#include "GameEventBus.h"
#include "GameSession.h"
#include "Scene.h"
#include "LevelsDatabase.h"

FrontendController::FrontendController()
    : mFrontendScreen(*this)
{
}

void FrontendController::OnOpenSinglePlayerMenuSelected()
{
    mFrontendScreen.ShowMenuPage(eFrontendMenuPage_SinglePlayer);
}

void FrontendController::OnMyPetDungeonMenuSelected()
{   
    mFrontendScreen.ShowMenuPage(eFrontendMenuPage_MyPetDungeon);
}

void FrontendController::OnMyPetDungeonMenuCancelled()
{
    mFrontendScreen.ShowMenuPage(eFrontendMenuPage_Main);
}

void FrontendController::OnMyPetDungeonLevelSelect(const std::string& fileName)
{
    ScenarioLevelInfo levelInfo;
    if (gLevelsDatabase.GetLevelInfo(fileName, levelInfo))
    {
        mFrontendScreen.ConfigureMissionBriefing(levelInfo);
        mFrontendScreen.ShowMenuPage(eFrontendMenuPage_MissionBriefing);
    }
    else
    {
        cxx_assert(false);
    }
}

void FrontendController::OnOpenSkirmishMenuSelected()
{
    mFrontendScreen.ShowMenuPage(eFrontendMenuPage_SkirmishMaps);
}

void FrontendController::OnSinglePlayerCancelled()
{
    mFrontendScreen.ShowMenuPage(eFrontendMenuPage_Main);
}

void FrontendController::OnSkirmishMapSelectCancelled()
{
    mFrontendScreen.ShowMenuPage(eFrontendMenuPage_SinglePlayer);
}

void FrontendController::OnSkirmishMapSelectConfirmed(const std::string& fileName)
{
    gGameEventBus.Send_StartScenarioRequest(fileName);  
}

void FrontendController::OnMissionBriefingCancelled(bool isMyPetDungeon)
{
    if (isMyPetDungeon)
    {
        mFrontendScreen.ShowMenuPage(eFrontendMenuPage_MyPetDungeon);
    }
    else
    {
        mFrontendScreen.ShowMenuPage(eFrontendMenuPage_Main);
    }
}

void FrontendController::OnMissionBriefingConfirmed(const std::string& fileName)
{
    gGameEventBus.Send_StartScenarioRequest(fileName);  
}

void FrontendController::OnQuitGameConfirmed()
{
    gGameEventBus.Send_QuitGameRequest();
}

void FrontendController::OnQuitGameCancelled()
{
    mFrontendScreen.ShowMenuPage(eFrontendMenuPage_Main);
}

void FrontendController::OnQuitGameSelected()
{
    mFrontendScreen.ShowMenuPage(eFrontendMenuPage_QuitGame);
}

void FrontendController::OnSessionLoaded()
{
    Player& localPlayer = gGameSession.GetLocalPlayer();

    // setup camera
    mCameraController.ResetCamera();

    glm::vec3 cameraTileCoord = MapUtils::ComputeTileCenter(localPlayer.GetStartCameraTilePosition());

    cameraTileCoord[1] = 1.65f; // height
    cameraTileCoord[2] -= 0.5f;
    mCameraController.SetStartPosition(cameraTileCoord);
    mCameraController.CaptureCamera(&gScene.GetCamera());
}

void FrontendController::OnSessionStart()
{
    if (mFrontendScreen.IsActive())
        return;

    // prepare screen

    mFrontendScreen.Activate();

    cxx::temp_vector<ScenarioLevelInfo> mapsList;
    mapsList.reserve(32);
    gLevelsDatabase.EnumSkirmishLevels([&mapsList](const ScenarioLevelInfo& levelInfo)
        {
            mapsList.push_back(levelInfo);
        });
    mFrontendScreen.ConfigureSkirmishMaps(mapsList);

    mapsList.clear();
    gLevelsDatabase.EnumMyPetDungeonLevels([&mapsList](const ScenarioLevelInfo& levelInfo)
        {
            mapsList.push_back(levelInfo);
        });
    mFrontendScreen.ConfigureMyPetDungeonMaps(mapsList);

    mFrontendScreen.ShowMenuPage(eFrontendMenuPage_Main);
}

void FrontendController::OnSessionShutdown()
{
    mCameraController.ReleaseCamera();
    if (mFrontendScreen.IsActive())
    {
        mFrontendScreen.Deactivate();
        mFrontendScreen.Cleanup();
    }
}

void FrontendController::UpdateFrame(float deltaTime)
{
    mCameraController.UpdateFrame(deltaTime);
}

void FrontendController::UpdateLogic(float stepDeltaTime)
{

}

void FrontendController::InputEvent(MouseButtonInputEvent& inputEvent)
{
    mCameraController.InputEvent(inputEvent);
}

void FrontendController::InputEvent(KeyInputEvent& inputEvent)
{
    mCameraController.InputEvent(inputEvent);
}

void FrontendController::InputEvent(MouseMovedInputEvent& inputEvent)
{
    mCameraController.InputEvent(inputEvent);
}

void FrontendController::InputEvent(MouseScrollInputEvent& inputEvent)
{
    mCameraController.InputEvent(inputEvent);
}