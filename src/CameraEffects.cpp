#include "stdafx.h"
#include "CameraEffects.h"
#include "DK2AssetLoader.h"
#include "Scene.h"

//////////////////////////////////////////////////////////////////////////
// Camera Sweep File
// https://github.com/ufdada/dk2-tools/blob/master/Formats/Camera/kcs_struct.bt
//////////////////////////////////////////////////////////////////////////

CameraEffects gCameraEffects;

//////////////////////////////////////////////////////////////////////////

static const std::string CameraPathFileExt = ".KCS";
static const std::string EnginePathFilePrefix = "enginepath";

//////////////////////////////////////////////////////////////////////////

CameraEffects::CameraEffects()
{

}

void CameraEffects::Initialize()
{
    DK2WADArchive* pathsArchive = gDK2AssetLoader.GetArchiveByName("Paths.WAD");
    cxx_assert(pathsArchive);

    if (pathsArchive == nullptr)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load cinematic camera paths");
        return;
    }

    ByteArray dataBuffer;
    pathsArchive->EnumEntries([this, &dataBuffer, pathsArchive](const std::string& entryName, DK2WADArchiveEntryID entryID)
        {
            if (!cxx::ends_with_icase(entryName, CameraPathFileExt))
                return;

            dataBuffer.clear();
            if (!pathsArchive->ExtractEntryData(entryID, dataBuffer))
            {
                cxx_assert(false);
                return;
            }
            cxx::memory_istream memorystream { (char*)dataBuffer.data(), (char*) dataBuffer.data() + dataBuffer.size()};
            std::istream instream(&memorystream);
            this->LoadCameraPath(entryName, instream);
        });
}

bool CameraEffects::LoadCameraPath(const std::string& name, std::istream& datastream)
{
    const unsigned int framesCount = cxx::read_int32(datastream);

    cxx_assert((framesCount > 0) && (framesCount < 0xFFFFFF));

    std::vector<CameraPath::Frame> cameraPathFrames;
    cameraPathFrames.resize(framesCount);

    // unused data
    cxx::read_int32(datastream);
    cxx::read_int32(datastream);
    cxx::read_int32(datastream);

    auto FloatsToVec3 = [](float ax, float ay, float az) -> glm::vec3
        {
            return glm::vec3{ax, -az, ay};
        };

    auto ReadVec3 = [&datastream, FloatsToVec3]() -> glm::vec3
        {
            float x = cxx::read_f32(datastream);
            float y = cxx::read_f32(datastream);
            float z = cxx::read_f32(datastream);
            return FloatsToVec3(x, y, z);
        };

    for (unsigned int iframe = 0; iframe < framesCount; ++iframe)
    {
        CameraPath::Frame& currFrame = cameraPathFrames[iframe];

        currFrame.mPosition = ReadVec3();
        currFrame.mForward  = ReadVec3();
        currFrame.mRight    = ReadVec3();
        currFrame.mUp       = ReadVec3();

        // i have no idea what i'm doing

        const glm::vec3 col0(  currFrame.mForward.x, -currFrame.mForward.y, -currFrame.mForward.z); 
        const glm::vec3 col1(  currFrame.mRight.x,   -currFrame.mRight.y,   -currFrame.mRight.z);   
        const glm::vec3 col2( -currFrame.mUp.x,       currFrame.mUp.y,       currFrame.mUp.z);      

        currFrame.mRight   = glm::normalize(col0);
        currFrame.mUp      = glm::normalize(col1);
        currFrame.mForward = glm::normalize(col2);

        // fov radians
        currFrame.mFov.set_angle(cxx::read_f32(datastream), cxx::angle_t::units::radians);

        // near
        cxx::read_f32(datastream);
        
        if (!datastream)
        {
            cxx_assert(false);
            return false;
        }
    }

    // todo: 1st_horny_path1, 2nd_horny_path1
    if (!cxx::starts_with_icase(name, EnginePathFilePrefix))
        return true;

    std::string_view numstr { 
        name.c_str() + EnginePathFilePrefix.length(), 
        name.length() - EnginePathFilePrefix.length() - CameraPathFileExt.length() 
    };
    unsigned int enginePathNumber {};
    if (!cxx::parse_int(numstr, enginePathNumber))
    {
        cxx_assert(false);
        return false;
    }

    cxx_assert(mCameraPaths.find(enginePathNumber) == mCameraPaths.end());

    mCameraPaths[enginePathNumber].mFrames = std::move(cameraPathFrames);
    return true;
}

void CameraEffects::UpdateFrame(float deltaTime)
{
    if (mOngoingTransition)
    {
        if (UpdateTransition(*mOngoingTransition, deltaTime))
        {
            // finished
            mOngoingTransition.reset();
        }
    }
}

bool CameraEffects::StartTransition(CameraPathId pathId, const glm::vec3& basePosition)
{
    mOngoingTransition.reset();

    // find path
    auto paths_it = mCameraPaths.find(pathId);
    if (paths_it == mCameraPaths.end())
    {
        cxx_assert(false);
        return false;
    }

    const float transitionFrameDuration = 1.0f / 24.0f;

    OngoingTransitionState& transitionState = mOngoingTransition.emplace();
    transitionState.mPathId = pathId;
    transitionState.mPathPtr = &paths_it->second;
    transitionState.mBasePosition = basePosition;
    transitionState.mProgressSeconds = 0.0f;
    transitionState.mDurationSeconds = transitionState.mPathPtr->GetFramesCount() * transitionFrameDuration;

    // apply camera state
    UpdateTransition(transitionState, 0.0f);
    return true;
}

void CameraEffects::StopTransition()
{
    mOngoingTransition.reset();
}

void CameraEffects::FinishTransition()
{
    if (mOngoingTransition)
    {
        OngoingTransitionState& transitionState = *mOngoingTransition;
        transitionState.mProgressSeconds = transitionState.mDurationSeconds;
        UpdateTransition(transitionState, 0.0f);
        mOngoingTransition.reset();
    }
}

bool CameraEffects::InTransition() const
{
    return mOngoingTransition.has_value();
}

void CameraEffects::PushCameraState()
{
    Camera& sceneCamera = gScene.GetCamera();

    SavedCameraState& cameraState = mCameraStateStack.emplace_back();
    cameraState.mProjection = sceneCamera.mProjectionParams;
    cameraState.mPosition = sceneCamera.mPosition;
    cameraState.mUp = sceneCamera.mUp;
    cameraState.mRight = sceneCamera.mRight;
    cameraState.mForward = sceneCamera.mForward;
}

void CameraEffects::RestoreCameraState()
{
    cxx_assert(!mCameraStateStack.empty());
    if (!mCameraStateStack.empty())
    {
        const SavedCameraState& cameraState = mCameraStateStack.back();

        Camera& sceneCamera = gScene.GetCamera();
        sceneCamera.ResetOrientation();
        sceneCamera.SetupProjection(cameraState.mProjection);
        sceneCamera.SetPosition(cameraState.mPosition);
        sceneCamera.mUp = cameraState.mUp;
        sceneCamera.mRight = cameraState.mRight;
        sceneCamera.mForward = cameraState.mForward;

        mCameraStateStack.pop_back();
    }
}

void CameraEffects::PopCameraState()
{
    cxx_assert(!mCameraStateStack.empty());
    if (!mCameraStateStack.empty())
    {
        mCameraStateStack.pop_back();
    }
}

bool CameraEffects::UpdateTransition(OngoingTransitionState& transitionState, float deltaTime)
{
    Camera& sceneCamera = gScene.GetCamera();

    transitionState.mProgressSeconds += deltaTime;

    const float t = std::min(transitionState.mProgressSeconds / transitionState.mDurationSeconds, 1.0f);
    CameraPath::Frame pathFrame = transitionState.mPathPtr->SampleFrameT(t);

    sceneCamera.ResetOrientation();
    sceneCamera.SetPosition(transitionState.mBasePosition + pathFrame.mPosition);
    sceneCamera.mUp = pathFrame.mUp;
    sceneCamera.mRight = pathFrame.mRight;
    sceneCamera.mForward = pathFrame.mForward;

    return (t >= 1.0f);
}
