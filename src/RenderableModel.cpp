#include "pch.h"
#include "RenderableModel.h"
#include "ModelAsset.h"
#include "TexturesManager.h"
#include "SceneObject.h"
#include "RenderManager.h"

RenderableModel::RenderableModel()
{
    mDebugColor = Color32_Green;
}

void RenderableModel::UpdateFrame(float deltaTime)
{
    if (!IsAnimationActive() || IsAnimationPaused())
        return;

    AdvanceAnimation(deltaTime);
}

void RenderableModel::PrepareRenderResources()
{
    AnimModelsRenderer& renderer = gRenderManager.mAnimatingModelsRenderer;
    renderer.PrepareRenderdata(this);
}

void RenderableModel::ReleaseRenderResources()
{
    AnimModelsRenderer& renderer = gRenderManager.mAnimatingModelsRenderer;
    renderer.ReleaseRenderdata(this);
}

void RenderableModel::RenderFrame(SceneRenderContext& renderContext)
{
    AnimModelsRenderer& renderer = gRenderManager.mAnimatingModelsRenderer;
    renderer.Render(renderContext, this);
}

void RenderableModel::SetModelAsset(ModelAsset* modelAsset)
{
    if (mModelAsset == modelAsset) // same asset, ignore
        return;

    Clear();

    if (modelAsset == nullptr)
        return;

    mModelAsset = modelAsset;

    const int NumMaterials = (int) modelAsset->mMaterialsArray.size();
    SetMeshMaterialsCount(NumMaterials);

    // setup materials
    mSubmeshTextures.clear();
    mSubmeshTextures.resize(NumMaterials);

    int iCurrentMaterial = 0;
    for (const ModelAsset::SubMeshMaterial& srcMaterial: modelAsset->mMaterialsArray)
    {
        MeshMaterial* material = GetMeshMaterial(iCurrentMaterial);
        debug_assert(material);

        debug_assert(!srcMaterial.mTextures.empty());

        material->mDiffuseTexture = gTexturesManager.GetTexture2D(srcMaterial.mTextures[0]);
        if (srcMaterial.mEnvMappingTexture.length())
        {
            material->mEnvMappingTexture = gTexturesManager.GetTexture2D(srcMaterial.mEnvMappingTexture);
        }

        if (srcMaterial.mFlagHasAlpha)
        {
            material->mRenderStates.mIsAlphaBlendEnabled = true;
            material->mRenderStates.mBlendingMode = eBlendingMode_Alpha;
        }

        if (srcMaterial.mFlagAlphaAdditive)
        {
            material->mRenderStates.mIsAlphaBlendEnabled = true;
            material->mRenderStates.mBlendingMode = eBlendingMode_AlphaAdditive;
            material->mRenderStates.mIsDepthWriteEnabled = false;
        }

        for (const std::string& sourceTexture: srcMaterial.mTextures)
        {
            mSubmeshTextures[iCurrentMaterial].push_back(gTexturesManager.GetTexture2D(sourceTexture));
        }
        ++iCurrentMaterial;
    }

    InvalidateMesh();
    SetAnimationState();
}

void RenderableModel::Clear()
{
    mModelAsset = nullptr;
    ReleaseRenderResources();
    ClearMeshMaterials();

    mSubmeshTextures.clear();

    SetAnimationState();
}

bool RenderableModel::StartAnimation(float animationSpeed, bool loop)
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

void RenderableModel::ClearAnimation()
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

void RenderableModel::RewindToStart()
{
    debug_assert(false); // todo
}

void RenderableModel::RewingToEnd()
{
    debug_assert(false); // todo
}

void RenderableModel::AdvanceAnimation(float deltaTime)
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

void RenderableModel::SetAnimationPaused(bool isPaused)
{
    if (IsAnimationActive())
    {
        mAnimState.mIsAnimationPaused = isPaused;
    }
}

bool RenderableModel::IsAnimationLoop() const
{
    return mAnimState.mIsAnimationLoop;
}

bool RenderableModel::IsAnimationActive() const
{
    return mAnimState.mIsAnimationActive;
}

bool RenderableModel::IsAnimationFinish() const
{
    return !mAnimState.mIsAnimationActive && mAnimState.mCyclesCount > 0;
}

bool RenderableModel::IsAnimationPaused() const
{
    return mAnimState.mIsAnimationPaused;
}

bool RenderableModel::IsStatic() const
{
    return mAnimState.mStartFrame == mAnimState.mFinalFrame;
}

void RenderableModel::SetPreferredLOD(int lod)
{
    if (mPreferredLOD == lod)
        return;

    mPreferredLOD = lod;
    // todo
}

void RenderableModel::SetAnimationState()
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

void RenderableModel::SetLocalBounds()
{
    SetLocalBoundingBox(mModelAsset->mFramesBounds[mAnimState.mFrame0]);
}
