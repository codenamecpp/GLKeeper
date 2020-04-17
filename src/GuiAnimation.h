#pragma once

#include "GuiDefs.h"

// base widgets animation class
class GuiAnimation: public cxx::noncopyable
{
public:
    // readonly
    const std::string mAnimationName;

public:
    GuiAnimation(const std::string& animationName);
    virtual ~GuiAnimation();

    // setup animation
    void InitAnimation(GuiWidget* animatingWidget);

    // read animation properties from json document
    void LoadProperties(cxx::json_document_node documentNode);

    // process widget animation
    // @param deltaTime: Time since last frame
    void AdvanceAnimation(float deltaTime);

    // play animation from current position
    void PlayAnimation(eGuiAnimationLoop animLoop);
    void PlayAnimationBackwards(eGuiAnimationLoop animLoop);

    // stop animation and leave widget in it's current state
    void StopAnimation();
    void StopAnimationAtEnd(); // disable current loop mode

    // instantly apply beginning or end animation state on widget
    void RewindToStart();
    void RewindToEnd();

    // test whether animation in progress
    bool IsAnimationActive() const; 
    bool IsRunsForwards() const;
    bool IsRunsBackwards() const;

protected:
    void AnimationStopped();

protected:
    // overridables
    virtual void HandleInitAnimation() {}
    virtual void HandleLoadProperties(cxx::json_document_node documentNode) {}
    virtual void HandleAnimationProgress(float progress) {}

protected:
    eGuiAnimationLoop mLoopMode = eGuiAnimationLoop_None;
    eGuiAnimationStatus mStatus = eGuiAnimationStatus_Stopped;
    int mCyclesCounter = 0;
    float mAnimationTime = 0.0f;
    float mDuration = 0.0f; // overall play time
    // animating widget is mandatory and so cannot be null
    GuiWidget* mAnimatingWidget = nullptr;
};