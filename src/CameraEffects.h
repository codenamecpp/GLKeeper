#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CameraPath.h"

//////////////////////////////////////////////////////////////////////////

class CameraEffects final: public cxx::noncopyable
{
private:

    //////////////////////////////////////////////////////////////////////////

    struct SavedCameraState
    {
        Camera::ProjectionParams mProjection {};
        glm::vec3 mPosition {};
        glm::vec3 mRight {};
        glm::vec3 mUp {};
        glm::vec3 mForward {};
    };

    //////////////////////////////////////////////////////////////////////////

    struct OngoingTransitionState
    {
        CameraPathId mPathId {};
        CameraPath* mPathPtr {};
        // camera path is always relative to a base position
        glm::vec3 mBasePosition {};
        float mProgressSeconds {};
        float mDurationSeconds {};
    };

    //////////////////////////////////////////////////////////////////////////

public:
    CameraEffects();

    void Initialize();
    void UpdateFrame(float deltaTime);

    // manage camera states
    void PushCameraState();
    void RestoreCameraState();
    void PopCameraState();

    // manage transition effect
    bool StartTransition(CameraPathId pathId, const glm::vec3& basePosition);
    void FinishTransition();
    void StopTransition();
    bool InTransition() const;

private:
    bool LoadCameraPath(const std::string& name, std::istream& datastream);
    void FixCameraPaths();

    CameraPath* GetCameraPath(CameraPathId pathId);

    bool UpdateTransition(OngoingTransitionState& transitionState, float deltaTime);

private:
    std::map<CameraPathId, CameraPath> mCameraPaths;

    std::vector<SavedCameraState> mCameraStateStack;

    std::optional<OngoingTransitionState> mOngoingTransition;
};

//////////////////////////////////////////////////////////////////////////

extern CameraEffects gCameraEffects;

//////////////////////////////////////////////////////////////////////////
