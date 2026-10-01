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
#include "AudioManager.h"
#include "SoundCategoryNames.h"
#include "UiManager.h"
#include "RoomManager.h"

FrontendController::FrontendController()
    : mFrontendUi(*this)
    , mCameraController(*this)
{
}

void FrontendController::OnOpenSinglePlayerMenuSelected()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_SinglePlayer);
}

void FrontendController::OnMyPetDungeonMenuSelected()
{   
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_MyPetDungeon);
    mFrontendUi.ShowMenuContent(false);
    mCameraController.StartTransitionToLocation(FrontendCameraController::eLocation_2ndLeft);
}

void FrontendController::OnMyPetDungeonMenuCancelled()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_Main);
    mFrontendUi.ShowMenuContent(false);
    mCameraController.StartTransitionToLocation(FrontendCameraController::eLocation_Entrance);
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
    mFrontendUi.ShowMenuContent(false);
    mCameraController.StartTransitionToLocation(FrontendCameraController::eLocation_1stRight);
}

void FrontendController::OnSinglePlayerCancelled()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_Main);
}

void FrontendController::OnSkirmishMapSelectCancelled()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_SinglePlayer);
    mFrontendUi.ShowMenuContent(false);
    mCameraController.StartTransitionToLocation(FrontendCameraController::eLocation_Entrance);
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

void FrontendController::OnNewCampaignSelected()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_CampaignTable);
    mFrontendUi.ShowMenuContent(false);
    mCameraController.StartTransitionToLocation(FrontendCameraController::eLocation_Table);
}

void FrontendController::OnContinueCampaignSelected()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_CampaignTable);
    mFrontendUi.ShowMenuContent(false);
    mCameraController.StartTransitionToLocation(FrontendCameraController::eLocation_Table);
}

void FrontendController::OnCampaignSelectionCancelled()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_SinglePlayer);
    mFrontendUi.ShowMenuContent(false);
    mCameraController.StartTransitionToLocation(FrontendCameraController::eLocation_Entrance);
}

void FrontendController::OnOpenExtrasMenuSelected()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_Extras);
    mFrontendUi.ShowMenuContent(false);
    mCameraController.StartTransitionToLocation(FrontendCameraController::eLocation_1stLeft);
}

void FrontendController::OnExtrasMenuConfirmed()
{
    mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_Main);
    mFrontendUi.ShowMenuContent(false);
    mCameraController.StartTransitionToLocation(FrontendCameraController::eLocation_Entrance);
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

void FrontendController::OnCameraTransitionCompleted()
{
    mFrontendUi.ShowMenuContent(true);
}

void FrontendController::UpdateCampaignTableInteractions(float deltaTime)
{
    EntityHandle currHoveredEntity {};

    if (!gUiManager.IsCursorOverUi())
    {
        const Point2D mouseScreenPos = gInputs.GetMousePosition();

        cxx::ray3d_t ray3d;
        if (gScene.CastRayFromScreenPoint(mouseScreenPos, ray3d))
        {
            cxx::temp_vector<RayHitResult> hitResults;
            if (gScene.QueryObjects(ray3d, hitResults))
            {
                // todo: not ideal
                std::sort(hitResults.begin(), hitResults.end(), 
                    [](const RayHitResult& lhs, const RayHitResult& rhs)
                    {
                        return lhs.mDistanceNear < rhs.mDistanceNear;
                    });

                for (const RayHitResult& hitsRoller: hitResults)
                {
                    const EntityHandle& entity = hitsRoller.mSceneObject->GetOwnerEntity();
                    if (!entity.IsRoom())
                        continue;

                } // for scene objects
            }
        } // if cast ray
    } 
}

void FrontendController::OnSessionLoaded()
{
    Player& localPlayer = gGameSession.GetLocalPlayer();

    // setup camera
    mCameraController.ResetCamera();

    glm::vec3 cameraTileCoord = MapUtils::ComputeTileCenter(localPlayer.GetStartCameraTilePosition());

    mCameraController.SetStartPosition(cameraTileCoord);
    mCameraController.CaptureCamera(&gScene.GetCamera());

    // find hero gate room
    for (Room* roller: gRoomManager.GetRoomsByType(RoomTypeId_HeroGate_Frontend))
    {
        cxx_assert(roller->ExistsOnMap());

        mHeroGateRoomHandle = roller->GetOwnHandle();
        break;
    }
    cxx_assert(mHeroGateRoomHandle.IsRoom());
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

    gAudio.PlayAmbience(SoundCategoryNames::Ambience, SoundGroupId_Ambience_Track);
}

void FrontendController::OnSessionShutdown()
{
    mCameraController.ReleaseCamera();
    if (mFrontendUi.IsActive())
    {
        mFrontendUi.Deactivate();
        mFrontendUi.Cleanup();
    }
    mHeroGateRoomHandle = {};
    gUiCursor.StateOff(UiCursor::eCursorState_PointOnThing);
}

void FrontendController::UpdateFrame(float deltaTime)
{
    mCameraController.UpdateFrame(deltaTime);

    if (mFrontendUi.IsOnMenuPage(FrontendUi::eMenuPage_CampaignTable) && 
        !mCameraController.InTransition())
    {
        UpdateCampaignTableInteractions(deltaTime);
    }
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