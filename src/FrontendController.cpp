#include "stdafx.h"
#include "FrontendController.h"
#include "GameWorld.h"
#include "GameMain.h"
#include "MapUtils.h"
#include "GameEventBus.h"
#include "GameSession.h"
#include "Scene.h"
#include "LevelsDatabase.h"
#include "UiCursor.h"

FrontendController::FrontendController()
    : mFrontendUi(*this)
{
}

void FrontendController::OnOpenSinglePlayerMenuSelected()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_SinglePlayer);
}

void FrontendController::OnMyPetDungeonMenuSelected()
{   
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_MyPetDungeon);
}

void FrontendController::OnMyPetDungeonMenuCancelled()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_Main);
}

void FrontendController::OnMyPetDungeonLevelSelect(const std::string& fileName)
{
    ScenarioLevelInfo levelInfo;
    if (gLevelsDatabase.GetLevelInfo(fileName, levelInfo))
    {
        mFrontendUi.ConfigureMissionBriefing(levelInfo);
        mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_MissionBriefing);
    }
    else
    {
        cxx_assert(false);
    }
}

void FrontendController::OnOpenSkirmishMenuSelected()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_SkirmishMaps);
}

void FrontendController::OnSinglePlayerCancelled()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_Main);
}

void FrontendController::OnSkirmishMapSelectCancelled()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_SinglePlayer);
}

void FrontendController::OnSkirmishMapSelectConfirmed(const std::string& fileName)
{
    gGameEventBus.Send_StartScenarioRequest(fileName);  
}

void FrontendController::OnMissionBriefingCancelled(bool isMyPetDungeon)
{
    if (isMyPetDungeon)
    {
        mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_MyPetDungeon);
    }
    else
    {
        mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_Main);
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
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_Main);
}

void FrontendController::OnQuitGameSelected()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_QuitGame);
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
    if (mFrontendUi.IsActive())
        return;

    gUiCursor.StateOn(UiCursor::eCursorState_PointOnThing);

    // prepare screen

    mFrontendUi.Activate();

    cxx::temp_vector<ScenarioLevelInfo> mapsList;
    mapsList.reserve(32);
    gLevelsDatabase.EnumSkirmishLevels([&mapsList](const ScenarioLevelInfo& levelInfo)
        {
            mapsList.push_back(levelInfo);
        });
    mFrontendUi.ConfigureSkirmishMaps(mapsList);

    mapsList.clear();
    gLevelsDatabase.EnumMyPetDungeonLevels([&mapsList](const ScenarioLevelInfo& levelInfo)
        {
            mapsList.push_back(levelInfo);
        });
    mFrontendUi.ConfigureMyPetDungeonMaps(mapsList);

    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_Main);
}

void FrontendController::OnSessionShutdown()
{
    mCameraController.ReleaseCamera();
    if (mFrontendUi.IsActive())
    {
        mFrontendUi.Deactivate();
        mFrontendUi.Cleanup();
    }

    gUiCursor.StateOff(UiCursor::eCursorState_PointOnThing);
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