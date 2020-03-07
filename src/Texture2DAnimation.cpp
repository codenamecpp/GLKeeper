#include "pch.h"
#include "Texture2DAnimation.h"

Texture2DAnimation::Texture2DAnimation(const std::string& animationName)
    : mAnimationName(animationName)
    , mFramesPerSecond()
    , mTimeAccumulator()
{
}

void Texture2DAnimation::UpdateAnimation(float deltaTime)
{
    
}

void Texture2DAnimation::RewindAnimation()
{

}
