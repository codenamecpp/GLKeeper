#include "pch.h"
#include "AnimModelComponent.h"
#include "ModelAsset.h"
#include "TexturesManager.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderManager.h"

AnimModelComponent::AnimModelComponent(GameObject* gameObject)
    : GameObjectComponent(eGameObjectComponent_AnimatingModel, gameObject)
{
    debug_assert(mGameObject);
    mGameObject->mDebugColor = Color32_Green;
}

AnimModelComponent::~AnimModelComponent()
{
}

void AnimModelComponent::UpdateFrame(float deltaTime)
{
    if (!IsAnimationActive() || IsAnimationPaused())
        return;

    AdvanceAnimation(deltaTime);
}

void AnimModelComponent::RenderFrame(SceneRenderContext& renderContext)
{
    AnimatingModelsRenderer& renderer = gRenderManager.mAnimatingModelsRenderer;
    renderer.Render(renderContext, this);
}

void AnimModelComponent::SetModelAsset(ModelAsset* modelAsset)
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
    mSubmeshMaterials.reserve(modelAsset->mMaterialsArray.size());

    mSubmeshTextures.clear();
    mSubmeshTextures.resize(modelAsset->mMaterialsArray.size());

    int iCurrentMaterial = 0;
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
            material.mRenderStates.mBlendingMode = eBlendingMode_AlphaAdditive;
            material.mRenderStates.mIsDepthWriteEnabled = false;
        }
        for (const std::string& sourceTexture: currentSourceMaterial.mTextures)
        {
            mSubmeshTextures[iCurrentMaterial].push_back(gTexturesManager.GetTexture2D(sourceTexture));
        }
        mSubmeshMaterials.push_back(material);
        ++iCurrentMaterial;
    }

    SetAnimationState();
}

void AnimModelComponent::SetModelAssetNull()
{
    mModelAsset = nullptr;
    // reset current renderdata
    if (mRenderData)
    {
        mRenderData = nullptr;
    }

    mSubmeshMaterials.clear();
    mSubmeshTextures.clear();

    SetAnimationState();
}

bool AnimModelComponent::StartAnimation(float animationSpeed, bool loop)
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

void AnimModelComponent::ClearAnimation()
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

    SetLocalBounds();
}

void AnimModelComponent::RewindToStart()
{
    debug_assert(false); // todo
}

void AnimModelComponent::RewingToEnd()
{
    debug_assert(false); // todo
}

void AnimModelComponent::AdvanceAnimation(float deltaTime)
{
    if (!IsAnimationActive())
    {
        debug_assert(false);
        return;
    }

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
        SetLocalBounds();
    }
}

void AnimModelComponent::SetAnimationPaused(bool isPaused)
{
    if (IsAnimationActive())
    {
        mAnimState.mIsAnimationPaused = isPaused;
    }
}

bool AnimModelComponent::IsAnimationLoop() const
{
    return mAnimState.mIsAnimationLoop;
}

bool AnimModelComponent::IsAnimationActive() const
{
    return mAnimState.mIsAnimationActive;
}

bool AnimModelComponent::IsAnimationFinish() const
{
    return !mAnimState.mIsAnimationActive && mAnimState.mCyclesCount > 0;
}

bool AnimModelComponent::IsAnimationPaused() const
{
    return mAnimState.mIsAnimationPaused;
}

bool AnimModelComponent::IsStatic() const
{
    return mAnimState.mStartFrame == mAnimState.mFinalFrame;
}

void AnimModelComponent::SetPreferredLOD(int lod)
{
    mPreferredLOD = lod;
}

void AnimModelComponent::SetAnimationState()
{   
    mAnimState = BlendFramesAnimState ();

    if (mModelAsset == nullptr)
    {
        return;
    }

    mAnimState.mStartFrame = 0;
    mAnimState.mFinalFrame = mModelAsset->mFramesCount - 1;

    SetLocalBounds();
}

void AnimModelComponent::SetLocalBounds()
{
    mGameObject->GetTransformComponent()->SetLocalBoundingBox(mModelAsset->mFramesBounds[mAnimState.mFrame0]);
}
