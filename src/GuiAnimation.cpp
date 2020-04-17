#include "pch.h"
#include "GuiAnimation.h"

GuiAnimation::GuiAnimation(const std::string& animationName)
    : mAnimationName(animationName)
{
}

GuiAnimation::~GuiAnimation()
{
}

void GuiAnimation::InitAnimation(GuiWidget* animatingWidget)
{
    debug_assert(animatingWidget);

    if (mAnimatingWidget == animatingWidget)
        return;

    mAnimatingWidget = animatingWidget;
    HandleInitAnimation();
}

void GuiAnimation::LoadProperties(cxx::json_document_node documentNode)
{
    if (!cxx::json_get_attribute(documentNode, "duration", mDuration))
    {
        debug_assert(false);
        mDuration = 1.0f;
    }

    HandleLoadProperties(documentNode);
}

void GuiAnimation::AdvanceAnimation(float deltaTime)
{
    if (mStatus == eGuiAnimationStatus_Stopped)
        return;

    // todo
}

void GuiAnimation::StopAnimation()
{
    mStatus = eGuiAnimationStatus_Stopped;

    AnimationStopped();
}

void GuiAnimation::StopAnimationAtEnd()
{
    mLoopMode = eGuiAnimationLoop_None;
}

void GuiAnimation::PlayAnimation(eGuiAnimationLoop animLoop)
{
    if (mStatus == eGuiAnimationStatus_PlayingForward)
        return;

    debug_assert(mAnimatingWidget);

    mAnimationTime = 0.0f;
    mCyclesCounter = 0;
    mStatus = eGuiAnimationStatus_PlayingForward;
    mLoopMode = animLoop;
}

void GuiAnimation::PlayAnimationBackwards(eGuiAnimationLoop animLoop)
{
    //if ()
}

void GuiAnimation::RewindToStart()
{
}

void GuiAnimation::RewindToEnd()
{
}

bool GuiAnimation::IsAnimationActive() const
{
    return mStatus != eGuiAnimationStatus_Stopped;
}

bool GuiAnimation::IsRunsForwards() const
{
    return mStatus == eGuiAnimationStatus_PlayingForward;
}

bool GuiAnimation::IsRunsBackwards() const
{
    return mStatus == eGuiAnimationStatus_PlayingBackward;
}

void GuiAnimation::AnimationStopped()
{
    // todo
}
