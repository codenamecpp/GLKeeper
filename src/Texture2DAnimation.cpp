#include "pch.h"
#include "Texture2DAnimation.h"
#include "Texture2D.h"

Texture2DAnimation::Texture2DAnimation()
    : mFramesPerSecond()
    , mTimeAccumulator()
    , mTimeScale(1.0f)
{
}

void Texture2DAnimation::AdvanceAnimation(float deltaTime)
{
    if (IsInitialized())
    {
        deltaTime = mTimeScale * deltaTime;

        if (deltaTime < 0.0001f) // threshold
            return;

        mTimeAccumulator += deltaTime;
        UpdateAnimatingTextureFrame();
    }
    else
    {
        debug_assert(false);
    }
}

void Texture2DAnimation::RewindToStart()
{
    mTimeAccumulator = 0.0f;
}

void Texture2DAnimation::SetAnimationSpeed(float framesPerSecond)
{
    mFramesPerSecond = framesPerSecond;
}

void Texture2DAnimation::SetAnimationTimeScale(float timeScale)
{
    mTimeScale = timeScale;
    debug_assert(mTimeScale >= 0.0f);
}

bool Texture2DAnimation::LoadFrameTextures()
{
    bool isSuccess = true;
    for (Texture2D* currTexture: mTextureFrames)
    {
        if (!currTexture->LoadTexture())
        {
            isSuccess = false;
        }
    }
    return isSuccess;
}

void Texture2DAnimation::FreeFrameTextures()
{
    for (Texture2D* currTexture: mTextureFrames)
    {
        if (currTexture->IsPersistent() || !currTexture->IsLoadedFromFile())
            continue;

        currTexture->FreeTexture();
    }
}

void Texture2DAnimation::SetFramesSamplerState(const TextureSamplerState& samplerState)
{
    for (Texture2D* currTexture: mTextureFrames)
    {
        currTexture->SetSamplerState(samplerState);
    }
}

void Texture2DAnimation::Clear()
{
    mTextureFrames.clear();
    
    mFramesPerSecond = 0.0f;
    mTimeAccumulator = 0.0f;
    mTimeScale = 1.0f;
    mAnimatingTexture = nullptr;
}

void Texture2DAnimation::AddAnimationFrame(Texture2D* texture)
{
    bool isInitialFrame = mTextureFrames.empty();

    mTextureFrames.push_back(texture);

    if (isInitialFrame)
    {
        UpdateAnimatingTextureFrame(0);
    }
}

void Texture2DAnimation::SetAnimatingTexture(Texture2D* animatingTexture)
{
    mAnimatingTexture = animatingTexture;

    if (mTextureFrames.size())
    {
        UpdateAnimatingTextureFrame(0);
    }
}

bool Texture2DAnimation::IsInitialized() const
{
    return mAnimatingTexture && mTextureFrames.size();
}

void Texture2DAnimation::UpdateAnimatingTextureFrame(int frame)
{
    if (mAnimatingTexture)
    {
        mAnimatingTexture->SetProxyTexture(mTextureFrames[frame]);
    }
}

void Texture2DAnimation::UpdateAnimatingTextureFrame()
{
    int animationFrame = GetCurrentFrameIndex();
    UpdateAnimatingTextureFrame(animationFrame);
}

int Texture2DAnimation::GetCurrentFrameIndex() const
{
    if (mTextureFrames.empty())
    {
        debug_assert(false);
        return 0;
    }

    const int NumAnimationFrames = mTextureFrames.size();

    float frameDuration = 1.0f / mFramesPerSecond;
    float animEndTime = frameDuration * NumAnimationFrames;

    float animCurrTime = ::fmodf(mTimeAccumulator, animEndTime);
    float animProgress = animCurrTime / animEndTime; // [0,1]

    int currentFrame = (int)(animProgress * NumAnimationFrames);
    debug_assert(currentFrame < NumAnimationFrames);
    return currentFrame;
}
