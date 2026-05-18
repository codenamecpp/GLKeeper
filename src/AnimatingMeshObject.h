#pragma once

//////////////////////////////////////////////////////////////////////////

#include "SceneObject.h"
#include "MeshAsset.h"

//////////////////////////////////////////////////////////////////////////

class AnimatingMeshObject: public SceneObject
{
    friend class AnimatingMeshRenderer;

public:
    
    //////////////////////////////////////////////////////////////////////////

    struct AnimationState
    {
    public:
        int mLoopCount = 0; // number of full animation cycles
        int mCurrFrame = 0; // base blend frame
        float mFramesMixFactor = 0.0f; // blend frames value in range [0..1]
        float mTime = 0.0f; // current animation cycle time
        bool mIsPaused = false;
    };

    //////////////////////////////////////////////////////////////////////////

    struct AnimationInfo
    {
    public:
        bool mHasAnimation = false;
        float mDuration = 0.0f; // duration of full animation cycle
    };

    //////////////////////////////////////////////////////////////////////////

public:
    AnimatingMeshObject();

    void Configure(MeshAsset* meshResource);
    void Configure(MeshAsset* meshResource, const AnimationParams& animParams);

    // animation control
    bool HasAnimation() const;
    bool HasAnimationCompleted() const;

    bool IsAnimationLooping() const;
    bool IsAnimationPaused() const;

    void PauseAnimation();
    void ResumeAnimation();
    void ResetAnimationProgress();
    void ResetAnimationSpeedFactor();

    float GetAnimationProgress() const;
    float GetAnimationSpeedFactor() const;
    float GetAnimationDuration() const;
    float GetAnimationEffectiveDuration() const;

    void SetAnimationSpeedFactor(float factor);
    void SetAnimationProgress(float progress);
    void SetAnimationLoopMode(eAnimationLoopMode loopMode);
    // will change current loop mode to eAnimationLoopMode_None
    void BreakAnimationLooping();

    void RewindToBegin();
    void RewindToEnd();

    int GetAnimationLoopCount() const;

    int GetAnimationFrameCount() const;
    int GetAnimationFrame() const;
    void SetAnimationFrame(int frame);

    // override SceneObject
    void UpdateFrame(float deltaTime) override;
    void RegisterForRendering(SceneRenderLists& renderLists, float distanceToCamera2) override;

    // pool
    void OnRecycle() override;

private:
    void ResetAnimation();
    void RefreshLocalBounds();

    void UpdateAnimationFrameFromProgress(float progress);
    int ComputeNextAnimationFrame() const;

private:
    MeshAsset* mMeshAsset = nullptr;
    AnimationState mAnimState;
    AnimationParams mAnimParams;
    AnimationInfo mAnimInfo;
};
