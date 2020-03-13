#include "pch.h"
#include "AnimatingModel.h"
#include "ModelAsset.h"
#include "TexturesManager.h"
#include "SceneRenderList.h"

AnimatingModel::AnimatingModel()
{
}

AnimatingModel::~AnimatingModel()
{
}

void AnimatingModel::SetModelAsset(ModelAsset* modelAsset)
{
    if (modelAsset == nullptr || !modelAsset->IsModelLoaded())
    {
        debug_assert(false);
        return;
    }

    mModelAsset = modelAsset;

    // setup bounding box
    SetLocalBoundingBox(mModelAsset->mFramesBounds[0]);

    // setup materials
    mSubmeshMaterials.clear();

    for (const ModelAsset::SubMeshMaterial& currentSourceMaterial: modelAsset->mMaterialsArray)
    {
        RenderMaterial material;
        if (currentSourceMaterial.mTextures.empty())
        {
            debug_assert(false);

            mSubmeshMaterials.push_back(material); // dummy material
            continue;
        }

        material.mDiffuseTexture = gTexturesManager.GetTexture2D(currentSourceMaterial.mTextures[0]);
        if (currentSourceMaterial.mEnvMappingTexture.length())
        {
            material.mEnvMappingTexture = gTexturesManager.GetTexture2D(currentSourceMaterial.mEnvMappingTexture);
        }

        if (currentSourceMaterial.mFlagHasAlpha)
        {
            material.mRenderStates.mIsAlphaBlendEnabled = true;
            material.mRenderStates.mBlendingMode = eBlendingMode_Alpha;
        }

        if (currentSourceMaterial.mFlagAlphaAdditive)
        {
            material.mRenderStates.mIsAlphaBlendEnabled = true;
            material.mRenderStates.mBlendingMode = eBlendingMode_Additive;
            material.mRenderStates.mIsDepthWriteEnabled = false;
        }

        mSubmeshMaterials.push_back(material);
    }

    SetAnimationState();
}

void AnimatingModel::SetModelAssetNull()
{
    mModelAsset = nullptr;

    mSubmeshMaterials.clear();

    SetAnimationState();
}

void AnimatingModel::UpdateFrame(float deltaTime)
{
    AdvanceAnimation(deltaTime);
}

void AnimatingModel::RegisterForRendering(SceneRenderList& renderList)
{
    // todo: check materials and add to proper render list

    renderList.RegisterRenderable(eRenderPass_Opaque, this);
}

bool AnimatingModel::StartAnimation(float animationSpeed, bool loop)
{
    if (IsStatic())
    {
        // static or null models cannot be animated
        return false;
    }

    ClearAnimation();

    debug_assert(mModelAsset);
    float frameDuration = 1.0f / animationSpeed;

    mAnimState.mAnimationEndTime = frameDuration * mModelAsset->mFramesCount;
    mAnimState.mFramesPerSecond = animationSpeed; 

    mAnimState.mIsAnimationActive = true;
    mAnimState.mIsAnimationLoop = loop;
    return true;
}

void AnimatingModel::ClearAnimation()
{
    mAnimState.mFrame0 = mAnimState.mStartFrame;
    mAnimState.mFrame1 = mAnimState.mStartFrame + 1;
    debug_assert(mAnimState.mFrame1 <= mAnimState.mFinalFrame);

    mAnimState.mCyclesCount = 0;

    mAnimState.mMixFrames = 0.0f;
    mAnimState.mAnimationTime = 0.0f;
    mAnimState.mAnimationEndTime = 0.0f;
    mAnimState.mFramesPerSecond = 0.0f;

    mAnimState.mIsAnimationActive = false;
    mAnimState.mIsAnimationLoop = false;
}

void AnimatingModel::RewindToStart()
{
    debug_assert(false); // todo
}

void AnimatingModel::RewingToEnd()
{
    debug_assert(false); // todo
}

void AnimatingModel::AdvanceAnimation(float deltaTime)
{
    if (!IsAnimationActive())
        return;

    debug_assert(mModelAsset);
    if (deltaTime < 0.0001f) // threshold
        return;

    mAnimState.mAnimationTime += deltaTime;

    if (mAnimState.mAnimationTime > mAnimState.mAnimationEndTime)
    {
        if (!mAnimState.mIsAnimationLoop)
        {
            mAnimState.mAnimationTime = mAnimState.mAnimationEndTime;
            ++mAnimState.mCyclesCount;
            mAnimState.mFrame0 = mAnimState.mFinalFrame;
            mAnimState.mFrame1 = mAnimState.mFinalFrame;
            mAnimState.mMixFrames = 0.0f;
            mAnimState.mIsAnimationActive = false;
            return;
        }

        // loop enabled
        mAnimState.mAnimationTime = ::fmodf(mAnimState.mAnimationTime, mAnimState.mAnimationEndTime);
    }

    float progress = (mAnimState.mAnimationTime / mAnimState.mAnimationEndTime); // [0,1]

    float baseFramef = glm::mix(mAnimState.mStartFrame * 1.0f, mAnimState.mFinalFrame * 1.0f, progress);
    mAnimState.mFrame0 = (int) baseFramef;
    mAnimState.mFrame1 = ((mAnimState.mFrame0 - mAnimState.mStartFrame + 1) % mModelAsset->mFramesCount) + mAnimState.mStartFrame;
    mAnimState.mMixFrames = (baseFramef - mAnimState.mFrame0 * 1.0f);
}

bool AnimatingModel::IsAnimationLoop() const
{
    return mAnimState.mIsAnimationLoop;
}

bool AnimatingModel::IsAnimationActive() const
{
    return mAnimState.mIsAnimationActive;
}

bool AnimatingModel::IsAnimationFinish() const
{
    return !mAnimState.mIsAnimationActive && mAnimState.mCyclesCount > 0;
}

bool AnimatingModel::IsStatic() const
{
    return mAnimState.mStartFrame == mAnimState.mFinalFrame;
}

void AnimatingModel::SetAnimationState()
{   
    mAnimState = BlendFramesAnimState ();

    if (mModelAsset == nullptr)
    {
        return;
    }

    mAnimState.mStartFrame = 0;
    mAnimState.mFinalFrame = mModelAsset->mFramesCount - 1;
}
