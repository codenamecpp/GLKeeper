#pragma once

#include "GraphicsDefs.h"

// forwards
class Texture2D;

// texture2d animation clip
class Texture2DAnimation: public cxx::noncopyable
{
public:
    // readonly
    std::string mAnimationName;

public:
    Texture2DAnimation(const std::string& animationName);

    // advance animation
    // @param deltaTime: Time passed since last update, seconds
    void UpdateAnimation(float deltaTime);

    // reset animation state
    void RewindAnimation();

private:
    std::vector<Texture2D*> mTextureFrames;
    float mFramesPerSecond;
    float mTimeAccumulator;
};