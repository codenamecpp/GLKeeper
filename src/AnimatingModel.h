#pragma once

#include "SceneDefs.h"
#include "ResourceDefs.h"
#include "Renderable.h"
#include "RenderDefs.h"

//////////////////////////////////////////////////////////////////////////

// model animation state
struct BlendFramesAnimState
{
public:
    // current animation params and state
    int mStartFrame = 0;
    int mFinalFrame = 0;

    int mFrame0 = 0; // base blend frame
    int mFrame1 = 0; // next blend frame

    int mCyclesCount = 0; // number of full animation cycles

    float mMixFrames = 0.0f; // blend frames value in range [0..1]
    float mAnimationTime = 0.0f; // current time progress
    float mAnimationEndTime = 0.0f;
    float mFramesPerSecond = 0.0f;  // speed

    bool mIsAnimationActive = false;
    bool mIsAnimationLoop = false;
    bool mIsAnimationPaused = false;
};

//////////////////////////////////////////////////////////////////////////

// model instance on render scene
class AnimatingModel: public Renderable
{
    friend class ModelsRenderer;

public:
    // readonly
    ModelAsset* mModelAsset = nullptr;

    std::vector<RenderMaterial> mSubmeshMaterials;
    BlendFramesAnimState mAnimState;

    int mPreferredLOD = 0;

public:
    AnimatingModel();
    ~AnimatingModel();

    // change model asset, setup bounds and materials
    // @param modelAsset: Source model data
    void SetModelAsset(ModelAsset* modelAsset);
    void SetModelAssetNull();

    // process scene update frame
    // @param deltaTime: Time since last update
    void UpdateFrame(float deltaTime) override;

    // request entity to register itself in render lists
    // @param renderPass: Current render pass
    // @param renderList: Render lists
    void RegisterForRendering(SceneRenderList& renderList) override;

    // model animation control
    bool StartAnimation(float animationSpeed, bool loop);
    void ClearAnimation();
    void RewindToStart();
    void RewingToEnd();
    void AdvanceAnimation(float deltaTime);
    void SetAnimationPaused(bool isPaused);

    bool IsAnimationLoop() const;
    bool IsAnimationActive() const;
    bool IsAnimationFinish() const;
    bool IsAnimationPaused() const;
    bool IsStatic() const;
   
    void SetPreferredLOD(int lod);

private:
    void SetAnimationState();

private:
    ModelsRenderData* mRenderData = nullptr;
};