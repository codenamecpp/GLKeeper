#pragma once

#include "GameObjectComponent.h"
#include "RenderDefs.h"
#include "ResourceDefs.h"

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

// animating model component of game object
class AnimModelComponent: public GameObjectComponent
{
public:
    // readonly
    ModelAsset* mModelAsset = nullptr;

    std::vector<RenderMaterial> mSubmeshMaterials;
    std::vector<std::vector<Texture2D*>> mSubmeshTextures; // additional textures
    BlendFramesAnimState mAnimState;

    int mPreferredLOD = 0;

    ModelsRenderData* mRenderData = nullptr;

public:
    AnimModelComponent(GameObject* gameObject);
    ~AnimModelComponent();

    // process component update frame
    // @param deltaTime: Time since last update
    void UpdateFrame(float deltaTime) override;

    // process render frame
    void RenderFrame(SceneRenderContext& renderContext) override;

    // change model asset, setup bounds and materials
    // @param modelAsset: Source model data
    void SetModelAsset(ModelAsset* modelAsset);
    void SetModelAssetNull();

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
    void SetLocalBounds();
};