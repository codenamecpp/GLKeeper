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
#include "EntityRegistry.h"

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
    else // main campaign
    {
        mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_CampaignTable);
        mFrontendUi.ShowMenuContent(false);
        mCameraController.StartTransitionToLocation(FrontendCameraController::eLocation_Table);
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

bool FrontendController::IsSelectingCampaignLevel() const
{
    return mFrontendUi.IsOnMenuPage(FrontendUi::eMenuPage_CampaignTable) && 
        !mCameraController.InTransition();
}

void FrontendController::UpdateCampaignLevelSelection(float deltaTime)
{
    if (gUiManager.IsCursorOverUi())
        return;

    const Point2D mouseScreenPos = gInputs.GetMousePosition();

    EntityHandle entityUnderCusror {};
    mSelectedCampaignLevel = {};

    cxx::temp_vector<RayHitResult> hitResults;
    cxx::ray3d_t ray3d;
    if (gScene.CastRayFromScreenPoint(mouseScreenPos, ray3d) && 
        gScene.QueryObjects(ray3d, hitResults))
    {
        for (auto hits_iter = hitResults.begin(); hits_iter != hitResults.end();)
        {
            RayHitResult& hitsRoller = *hits_iter;
            if (!hitsRoller.mSceneObject->Raycast(ray3d, hitsRoller.mDistanceNear))
            {
                hits_iter = hitResults.erase(hits_iter);
                continue;
            }
            ++hits_iter;
        }
        std::sort(hitResults.begin(), hitResults.end(), 
            [](const RayHitResult& lhs, const RayHitResult& rhs)
            {
                return lhs.mDistanceNear < rhs.mDistanceNear;
            });
    }

    if (auto roomCapability = gEntities.GetEntityCapability<I3DMapTableRoom>(mHeroGateRoomHandle))
    {
        if (!hitResults.empty())
        {
            const RayHitResult& rayHitResult = hitResults.front();
            const EntityHandle entityHandle = rayHitResult.mSceneObject->GetOwnerEntity();
            if (roomCapability->SelectLevel(entityHandle))
            {
                entityUnderCusror = entityHandle;
                if (!roomCapability->GetLevelKey(entityUnderCusror, mSelectedCampaignLevel))
                {
                    cxx_assert(false);
                }
            }
        }

        if (!entityUnderCusror)
        {
            roomCapability->DeselectLevel();
        }
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
    mSelectedCampaignLevel = {};
    gUiCursor.StateOff(UiCursor::eCursorState_PointOnThing);
}

void FrontendController::UpdateFrame(float deltaTime)
{
    mCameraController.UpdateFrame(deltaTime);

    if (IsSelectingCampaignLevel())
    {
        UpdateCampaignLevelSelection(deltaTime);
    }
}

void FrontendController::UpdateLogic(float stepDeltaTime)
{

}

void FrontendController::InputEvent(MouseButtonInputEvent& inputEvent)
{
    mCameraController.InputEvent(inputEvent);

    if (IsSelectingCampaignLevel())
    {
        if (inputEvent.IsButtonPressed(MBUTTON_LEFT) && mSelectedCampaignLevel)
        {
            ScenarioLevelInfo scenarioLevelInfo {};
            if (gLevelsDatabase.GetCampaignLevelInfo(mSelectedCampaignLevel, scenarioLevelInfo))
            {
                mFrontendUi.ConfigureMissionBriefing(scenarioLevelInfo);
                mFrontendUi.ShowMenuPage(FrontendUi::eMenuPage_MissionBriefing);
                mFrontendUi.ShowMenuContent(false);
                mCameraController.StartTransitionToLocation(FrontendCameraController::eLocation_1stRight);
            }
            else
            {
                cxx_assert(false);
            }
        } 
    }
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