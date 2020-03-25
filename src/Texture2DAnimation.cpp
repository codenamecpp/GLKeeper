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
    deltaTime = mTimeScale * deltaTime;

    if (deltaTime < 0.0001f) // threshold
        return;

    mTimeAccumulator += deltaTime;
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

bool Texture2DAnimation::ActivateFrameTexture(eTextureUnit textureUnit)
{
    Texture2D* currentFrame = GetCurrentFrameTexture();
    if (currentFrame == nullptr)
    {
        debug_assert(false);
        return false;
    }
    return currentFrame->ActivateTexture(textureUnit);
}

bool Texture2DAnimation::IsFrameTextureActivate(eTextureUnit textureUnit) const
{
    Texture2D* currentFrame = GetCurrentFrameTexture();
    if (currentFrame == nullptr)
    {
        debug_assert(false);
        return false;
    }
    return currentFrame->IsTextureActivate(textureUnit);
}

bool Texture2DAnimation::IsFrameTextureActivate() const
{
    Texture2D* currentFrame = GetCurrentFrameTexture();
    if (currentFrame == nullptr)
    {
        debug_assert(false);
        return false;
    }
    return currentFrame->IsTextureActivate();
}

void Texture2DAnimation::Clear()
{
    mTextureFrames.clear();
    
    mFramesPerSecond = 0.0f;
    mTimeAccumulator = 0.0f;
    mTimeScale = 1.0f;
}

void Texture2DAnimation::AddAnimationFrame(Texture2D* texture)
{
    mTextureFrames.push_back(texture);
}

Texture2D* Texture2DAnimation::GetCurrentFrameTexture() const
{
    if (mTextureFrames.empty())
        return nullptr;

    const int NumAnimationFrames = mTextureFrames.size();

    float frameDuration = 1.0f / mFramesPerSecond;
    float animEndTime = frameDuration * NumAnimationFrames;

    float animCurrTime = ::fmodf(mTimeAccumulator, animEndTime);
    float animProgress = animCurrTime / animEndTime; // [0,1]

    int currentFrame = (int)(animProgress * NumAnimationFrames);
    debug_assert(currentFrame < NumAnimationFrames);
    return mTextureFrames[currentFrame];
}
