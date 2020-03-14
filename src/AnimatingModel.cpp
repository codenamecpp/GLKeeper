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

    if (mModelAsset == modelAsset) // same asset, ignore
        return;

    mModelAsset = modelAsset;

    // reset current renderdata
    if (mRenderData)
    {
        mRenderData = nullptr;
    }

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

        if (currentSourceMaterial.mFlagAlphaAdditive || currentSourceMaterial.mFlagShinyness)
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
    // reset current renderdata
    if (mRenderData)
    {
        mRenderData = nullptr;
    }

    mSubmeshMaterials.clear();

    SetAnimationState();
}

void AnimatingModel::UpdateFrame(float deltaTime)
{
    if (IsAnimationPaused())
        return;

    AdvanceAnimation(deltaTime);
}

void AnimatingModel::RegisterForRendering(SceneRenderList& renderList)
{
    if (mModelAsset == nullptr)
        return;
    
    bool hasOpaqueParts = false;
    bool hasTranslucentParts = false;

    for (const RenderMaterial& currentMaterial: mSubmeshMaterials)
    {
        if (currentMaterial.IsTransparent())
        {
            hasTranslucentParts = true;
        }
        else
        {
            hasOpaqueParts = true;
        }
    }

    if (hasOpaqueParts)
    {
        renderList.RegisterRenderable(eRenderPass_Opaque, this);
    }

    if (hasTranslucentParts)
    {
        renderList.RegisterRenderable(eRenderPass_Translucent, this);
    }
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
    mAnimState.mIsAnimationPaused = false;

    SetLocalBoundingBox(mModelAsset->mFramesBounds[mAnimState.mFrame0]);
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

        float numCycleTimes = (mAnimState.mAnimationTime / mAnimState.mAnimationEndTime);
        mAnimState.mCyclesCount += (int) numCycleTimes;

        // loop enabled
        mAnimState.mAnimationTime = ::fmodf(mAnimState.mAnimationTime, mAnimState.mAnimationEndTime);
    }

    float progress = (mAnimState.mAnimationTime / mAnimState.mAnimationEndTime); // [0,1]

    float baseFramef = glm::mix(mAnimState.mStartFrame * 1.0f, mAnimState.mFinalFrame * 1.0f, progress);

    int prevFrame = mAnimState.mFrame0;
    mAnimState.mFrame0 = (int) baseFramef;
    mAnimState.mFrame1 = ((mAnimState.mFrame0 - mAnimState.mStartFrame + 1) % mModelAsset->mFramesCount) + mAnimState.mStartFrame;
    mAnimState.mMixFrames = (baseFramef - mAnimState.mFrame0 * 1.0f);

    if (prevFrame != mAnimState.mFrame0)
    {
        SetLocalBoundingBox(mModelAsset->mFramesBounds[mAnimState.mFrame0]);
    }
}

void AnimatingModel::SetAnimationPaused(bool isPaused)
{
    if (IsAnimationActive())
    {
        mAnimState.mIsAnimationPaused = isPaused;
    }
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

bool AnimatingModel::IsAnimationPaused() const
{
    return mAnimState.mIsAnimationPaused;
}

bool AnimatingModel::IsStatic() const
{
    return mAnimState.mStartFrame == mAnimState.mFinalFrame;
}

void AnimatingModel::SetPreferredLOD(int lod)
{
    mPreferredLOD = lod;
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

    // setup bounding box
    SetLocalBoundingBox(mModelAsset->mFramesBounds[mAnimState.mFrame0]);
}
