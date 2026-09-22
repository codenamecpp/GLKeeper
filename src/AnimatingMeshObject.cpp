#include "stdafx.h"
#include "AnimatingMeshObject.h"

AnimatingMeshObject::AnimatingMeshObject()
    : SceneObject(eSceneObjectType_AnimatingMesh)
{
}

void AnimatingMeshObject::Configure(MeshAsset* meshResource)
{
    mMeshAsset = meshResource;

    ResetAnimation();
    RefreshLocalBounds();
}

void AnimatingMeshObject::Configure(MeshAsset* meshResource, const AnimationParams& animParams)
{
    mMeshAsset = meshResource;
    cxx_assert(mMeshAsset);

    ResetAnimation();
    if (mMeshAsset)
    {   
        mAnimParams = animParams;

        int genuineFirstFrame = 0;
        int genuineLastFrame = (mMeshAsset->GetAnimFramesCount() > 1) ? mMeshAsset->GetAnimFramesCount() - 1 : 0;

        mAnimParams.mFirstFrame = std::clamp(mAnimParams.mFirstFrame, genuineFirstFrame, genuineLastFrame);
        mAnimParams.mLastFrame = std::clamp(mAnimParams.mLastFrame, mAnimParams.mFirstFrame, genuineLastFrame);
        mAnimParams.mFramesPerSecond = std::max(0.0f, mAnimParams.mFramesPerSecond);

        // configure anim info
        mAnimInfo.mHasAnimation = (mAnimParams.mFramesPerSecond > 0.0f);
        if (HasAnimation())
        {
            mAnimInfo.mDuration = GetAnimationFrameCount() / mAnimParams.mFramesPerSecond;
            cxx_assert(mAnimInfo.mDuration > 0.0f);
        }

        // initial animation state
        ResetAnimationProgress();
    }
    RefreshLocalBounds();
}

bool AnimatingMeshObject::HasAnimation() const
{
    return mAnimInfo.mHasAnimation;
}

bool AnimatingMeshObject::HasAnimationCompleted() const
{
    return HasAnimation() && (mAnimParams.mLoopMode == eAnimationLoopMode_None) && (mAnimState.mLoopCount > 0);
}

bool AnimatingMeshObject::IsAnimationLooping() const
{
    return HasAnimation() && (mAnimParams.mLoopMode != eAnimationLoopMode_None);
}

bool AnimatingMeshObject::IsAnimationPaused() const
{
    return mAnimState.mIsPaused;
}

void AnimatingMeshObject::PauseAnimation()
{
    if (HasAnimation())
    {
        mAnimState.mIsPaused = true;
    }
}

void AnimatingMeshObject::ResumeAnimation()
{
    if (HasAnimation())
    {
        mAnimState.mIsPaused = false;
    }
}

void AnimatingMeshObject::ResetAnimationProgress()
{
    if (HasAnimation() && ((mAnimState.mTime > 0.0f) || mAnimState.mLoopCount > 0))
    {
        mAnimState.mLoopCount = 0;
        mAnimState.mCurrFrame = mAnimParams.mFirstFrame;
        mAnimState.mFramesMixFactor = 0.0f;
        mAnimState.mTime = 0.0f;

        RefreshLocalBounds();
    }
}

void AnimatingMeshObject::ResetAnimationSpeedFactor()
{
    mAnimParams.mSpeedFactor = 1.0f;
}

float AnimatingMeshObject::GetAnimationProgress() const
{
    float currentProgress = 0.0f;
    if (HasAnimation())
    {
        const float t = (mAnimState.mTime / mAnimInfo.mDuration);
        currentProgress = std::clamp(t, 0.0f, 1.0f);
    }
    return currentProgress;
}

float AnimatingMeshObject::GetAnimationSpeedFactor() const
{
    return mAnimParams.mSpeedFactor;
}

float AnimatingMeshObject::GetAnimationDuration() const
{
    return mAnimInfo.mDuration;
}

float AnimatingMeshObject::GetAnimationEffectiveDuration() const
{
    return mAnimInfo.mDuration * mAnimParams.mSpeedFactor;
}

void AnimatingMeshObject::SetAnimationSpeedFactor(float factor)
{
    if (HasAnimation())
    {
        mAnimParams.mSpeedFactor = std::max(factor, 0.0f);
    }
}

void AnimatingMeshObject::SetAnimationProgress(float progress)
{
    if (HasAnimation())
    {
        progress = std::clamp(progress, 0.0f, 1.0f);
        if (progress == GetAnimationProgress())
            return;

        mAnimState.mTime = std::clamp(progress * mAnimInfo.mDuration, 0.0f, mAnimInfo.mDuration);
        UpdateAnimationFrameFromProgress(progress);
    }
}

void AnimatingMeshObject::SetAnimationLoopMode(eAnimationLoopMode loopMode)
{
    if (HasAnimation())
    {
        if (mAnimParams.mLoopMode == loopMode) return;

        mAnimParams.mLoopMode = loopMode;
        mAnimState.mLoopCount = 0; // force reset loop count
    }
}

void AnimatingMeshObject::BreakAnimationLooping()
{
    if (IsAnimationLooping())
    {
        SetAnimationLoopMode(eAnimationLoopMode_None);
    }
}

void AnimatingMeshObject::RewindToBegin()
{
    SetAnimationProgress(0.0f);
}

void AnimatingMeshObject::RewindToEnd()
{
    SetAnimationProgress(1.0f);
}

int AnimatingMeshObject::GetAnimationLoopCount() const
{
    return mAnimState.mLoopCount;
}

int AnimatingMeshObject::GetAnimationFrameCount() const
{
    return (mAnimParams.mLastFrame - mAnimParams.mFirstFrame) + 1;
}

int AnimatingMeshObject::GetAnimationFrame() const
{
    return mAnimState.mCurrFrame;
}

void AnimatingMeshObject::SetAnimationFrame(int frame)
{
    if (!HasAnimation()) return;

    const int prevFrame = mAnimState.mCurrFrame;
    mAnimState.mCurrFrame = std::clamp(frame, mAnimParams.mFirstFrame, mAnimParams.mLastFrame);
    mAnimState.mFramesMixFactor = 0.0f;
    if (prevFrame != mAnimState.mCurrFrame)
    {
        RefreshLocalBounds();
    }
}

void AnimatingMeshObject::SetHighlighted(bool isHighlighted)
{
    mIsHighlighted = isHighlighted;
}

void AnimatingMeshObject::UpdateFrame(float deltaTime)
{
    if (HasAnimation())
    {
        if (HasAnimationCompleted() || IsAnimationPaused()) 
            return;

        mAnimState.mTime += (deltaTime * mAnimParams.mSpeedFactor);
        // end animation cycle?
        if (mAnimState.mTime >= mAnimInfo.mDuration)
        {   
            // start over?
            if (IsAnimationLooping())
            {
                do 
                {
                    mAnimState.mTime -= mAnimInfo.mDuration;
                    mAnimState.mLoopCount++;
                } 
                while (mAnimState.mTime >= mAnimInfo.mDuration);
            }
            // complete?
            else
            {
                mAnimState.mTime = mAnimInfo.mDuration;
                mAnimState.mLoopCount++;
            }
        }
        
        // compute current frame
        UpdateAnimationFrameFromProgress(mAnimState.mTime / mAnimInfo.mDuration);
    }
}

void AnimatingMeshObject::RegisterForRendering(SceneRenderLists& renderLists, float distanceToCamera2)
{
    if (mMeshAsset == nullptr) return;

    bool hasOpaqueParts = false;
    bool hasTranslucentParts = false;

    for (const SurfaceMaterial& currentMaterial: mMeshAsset->GetRenderMaterialsList())
    {
        if (currentMaterial.IsOpaque())
        {
            hasOpaqueParts = true;
        }
        else
        {
            hasTranslucentParts = true;
        }
    }

    if (hasOpaqueParts)
    {
        renderLists.Register(eRenderPass_Opaque, this, GetSceneObjectTypeId(),  distanceToCamera2);
    }

    if (hasTranslucentParts)
    {
        renderLists.Register(eRenderPass_Translucent, this, GetSceneObjectTypeId(), distanceToCamera2);
    }
}

void AnimatingMeshObject::OnRecycle()
{
    SceneObject::OnRecycle();

    ResetAnimation();
    mMeshAsset = nullptr;

    mIsHighlighted = false;
}

void AnimatingMeshObject::ResetAnimation()
{
    mAnimParams = {};
    mAnimInfo = {};
    mAnimState = {};
}

void AnimatingMeshObject::RefreshLocalBounds()
{
    cxx::aabbox bounds;
    bounds.set_to_zero();
    if (mMeshAsset)
    {
#if 1
        bounds = mMeshAsset->GetMaxBounds();
#else
        bounds = mMeshAsset->GetBounds(mAnimState.mCurrFrame);
#endif
    }
    SetLocalBoundingBox(bounds);
}

void AnimatingMeshObject::UpdateAnimationFrameFromProgress(float progress)
{
    const int prevFrame = mAnimState.mCurrFrame;

    progress = std::clamp(progress, 0.0f, 1.0f);

    const float baseFramef = progress * GetAnimationFrameCount();
    mAnimState.mCurrFrame = std::clamp(static_cast<int>(baseFramef), mAnimParams.mFirstFrame, mAnimParams.mLastFrame);
    mAnimState.mFramesMixFactor = glm::fract(baseFramef);

    if (prevFrame != mAnimState.mCurrFrame)
    {
        RefreshLocalBounds();
    }
}

int AnimatingMeshObject::ComputeNextAnimationFrame() const
{
    int nextFrame = mAnimState.mCurrFrame + 1;
    if (nextFrame > mAnimParams.mLastFrame)
    {
        nextFrame = IsAnimationLooping() ? mAnimParams.mFirstFrame : mAnimParams.mLastFrame;
    }
    return nextFrame;
}
