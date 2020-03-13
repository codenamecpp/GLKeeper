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

    int mFrame0 = 0;
    int mFrame1 = 0;

    int mCyclesCount = 0; // number of full animation cycles

    float mMixFrames = 0.0f; // value in range [0..1]
    float mAnimationTime = 0.0f;
    float mAnimationEndTime = 0.0f;
    float mFramesPerSecond = 0.0f; // speed

    bool mIsAnimationActive = false;
    bool mIsAnimationLoop = false;
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

public:
    AnimatingModel();
    ~AnimatingModel();

    // change model asset, setup bounds and materials
    // @param modelAsset: Source model data
    void SetModelAsset(ModelAsset* modelAsset);
    void SetModelAssetNull();

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

    bool IsAnimationLoop() const;
    bool IsAnimationActive() const;
    bool IsAnimationFinish() const;
    bool IsStatic() const;

private:
    void SetAnimationState();

private:
    ModelsRenderData* mRenderData = nullptr;
};