#pragma once

#include "GraphicsDefs.h"
#include "ResourceDefs.h"

// texture2d animation clip
class Texture2DAnimation: public cxx::noncopyable
{
public:
    // readonly
    std::vector<Texture2D*> mTextureFrames;
    Texture2D* mAnimatingTexture = nullptr;

public:
    Texture2DAnimation();

    // advance animation
    // @param deltaTime: Time passed since last update, seconds
    void AdvanceAnimation(float deltaTime);

    // reset animation state
    void RewindToStart();

    void SetAnimationSpeed(float framesPerSecond);
    void SetAnimationTimeScale(float timeScale);

    bool LoadFrameTextures();
    void FreeFrameTextures();
    void SetFramesSamplerState(const TextureSamplerState& samplerState);

    void Clear();
    void AddAnimationFrame(Texture2D* texture);

    void SetAnimatingTexture(Texture2D* animatingTexture);

    bool IsInitialized() const;

private:
    void UpdateAnimatingTextureFrame(int frame);
    void UpdateAnimatingTextureFrame();
    int GetCurrentFrameIndex() const;

private:
    float mFramesPerSecond;
    float mTimeAccumulator;
    float mTimeScale;
};