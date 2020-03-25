#pragma once

#include "GraphicsDefs.h"
#include "ResourceDefs.h"

// texture2d animation clip
class Texture2DAnimation: public cxx::noncopyable
{
public:
    // readonly
    std::vector<Texture2D*> mTextureFrames;

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

    bool ActivateFrameTexture(eTextureUnit textureUnit);
    bool IsFrameTextureActivate(eTextureUnit textureUnit) const;
    bool IsFrameTextureActivate() const; 

    void Clear();
    void AddAnimationFrame(Texture2D* texture);

private:
    Texture2D* GetCurrentFrameTexture() const;

private:
    float mFramesPerSecond;
    float mTimeAccumulator;
    float mTimeScale;
};