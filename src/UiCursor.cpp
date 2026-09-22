#include "stdafx.h"
#include "UiCursor.h"
#include "TextureManager.h"
#include "UiRenderContext.h"

//////////////////////////////////////////////////////////////////////////

UiCursor gUiCursor;

//////////////////////////////////////////////////////////////////////////

void UiCursor::Init()
{
    InitCursorState(eCursorState_PointOnThing, "Idle.png", {0, 3}, 1);
    InitCursorState(eCursorState_HoldGold, "HoldGold.png", {-18, 6}, 1);
    InitCursorState(eCursorState_HoldThing, "HoldThing.png", {8, 42}, 1);
    InitCursorState(eCursorState_DropGold, "DropGold.png", {4, 58}, 16);
    InitCursorState(eCursorState_DropThing, "DropThing.png", {0, 63}, 14);
    InitCursorState(eCursorState_PickAxeHold, "PickAxeHold.png", {2, 48}, 1);
    InitCursorState(eCursorState_PickAxeTag, "PickAxeTag.png", {6, 30}, 1);
    InitCursorState(eCursorState_Idle, "Point.png", {2, 18}, 41);
    InitCursorState(eCursorState_Slap, "Slap.png", {-2, 64}, 15);
    //InitCursorState(eCursorState_PointOnUi, "SmallPoint.png", {0, 2}, 1);
    mStates[eCursorState_PointOnUi] = mStates[eCursorState_PointOnThing];
    InitCursorState(eCursorState_SpellCast, "SpellCast.png", {0, 64}, 12);
    InitCursorState(eCursorState_SpellHold, "SpellHold.png", {-20, 64}, 1);

    mMousePosition = gInputs.GetMousePosition();

    mOneShotAnimationSet.Clear();
    mOneShotAnimationSet.Include(eCursorState_DropGold)
        .Include(eCursorState_DropThing)
        .Include(eCursorState_Slap)
        .Include(eCursorState_SpellCast);
    UpdateState();
}

void UiCursor::Deinit()
{
    for (CursorState& roller: mStates)
    {
        roller.mAnimationFrames.clear();
        roller.mHotSpot = {};
        roller.mTexture = {};
    }
    mCurrentStateId = eCursorState_Hidden;
    mAnimFrameTimer = {};
    mAnimFrameIndex = 0;
    mLastStateSet = {};
    mNextStateSet = {};
    mAnimCycleDone = {};
}

void UiCursor::UpdateFrame(float deltaTime)
{
    mMousePosition = gInputs.GetMousePosition();

    // state animation
    if (mCurrentStateId != eCursorState_Hidden)
    {
        UpdateStateAnimation(deltaTime);
    }

    // state flags
    UpdateState();
}

void UiCursor::RenderFrame(UiRenderContext& renderContext)
{
    if (mCurrentStateId == eCursorState_Hidden)
        return;

    const CursorState& cursorState = mStates[mCurrentStateId];
    const Rect2D& srcRect = cursorState.mAnimationFrames[mAnimFrameIndex];
    Rect2D dstRect = srcRect;
    dstRect.SetPosition(mMousePosition - cursorState.mHotSpot);
    renderContext.DrawTexture(cursorState.mTexture, COLOR_WHITE, dstRect, srcRect);
}

void UiCursor::InitCursorState(eCursorState stateId, const std::string& textureName, const Point2D& hotSpot, int frameCount)
{
    cxx_assert(stateId != eCursorState_Hidden);
    cxx_assert(stateId < eCursorState_COUNT);

    CursorState& cursorState = mStates[stateId];

    cursorState.mTexture = gTextureManager.GetTexture(textureName, eTextureBacking_None);
    
    cxx_assert(frameCount > 0);
    const TextureRegion& textureRegion = cursorState.mTexture->GetTextureRegion();
    const Point2D imageSize = textureRegion.mRect.GetSize();
    const Point2D frameSize = { imageSize.x, imageSize.y / frameCount };

    cursorState.mAnimationFrames.clear();
    cursorState.mAnimationFrames.resize(frameCount);
    for (int iframe = 0; iframe < frameCount; ++iframe)
    {
        Rect2D& rcFrame = cursorState.mAnimationFrames[iframe];
        rcFrame.SetPosition({0, iframe * frameSize.y});
        rcFrame.SetSize(frameSize);
    }
    cursorState.mHotSpot = hotSpot;
}

void UiCursor::UpdateStateAnimation(float deltaTime)
{
    if (!mAnimFrameTimer.IsStarted() || 
        !mAnimFrameTimer.TickAndCheckExpire(deltaTime))
    {
        return;
    }

    CursorState& animation = mStates[mCurrentStateId];
    bool isLastAnimationFrame = (mAnimFrameIndex + 1) >= static_cast<int>(animation.mAnimationFrames.size());
    if (isLastAnimationFrame)
    {
        mAnimCycleDone = true;
        return;
    }

    ++mAnimFrameIndex;
    mAnimFrameTimer.Restart();
}

UiCursor::eCursorState UiCursor::DetectNextState() const
{
    if (!mNextStateSet.Empty())
    {
        for (int istate = 0; istate < eCursorState_COUNT; ++istate)
        {
            const eCursorState statesRoller = static_cast<eCursorState>(istate);
            if (mNextStateSet.Contains(statesRoller))
            {
                return statesRoller;
            }
        }
    }
    return eCursorState_Idle;
}

bool UiCursor::SelectNextState()
{
    const eCursorState nextStateId = DetectNextState();
    if (CanChangeState(nextStateId))
    {
        ChangeState(nextStateId);
        return true;
    }
    return false;
}

void UiCursor::ChangeState(eCursorState stateId)
{
    mCurrentStateId = stateId;
    mAnimFrameTimer = {};
    mAnimFrameIndex = 0;
    if (mCurrentStateId != eCursorState_Hidden)
    {
        mAnimFrameTimer.Start(1.0f / 30.0f);
    }
    mAnimCycleDone = {};
}

void UiCursor::StateOn(eCursorState stateId)
{
    mNextStateSet.Include(stateId);
}

void UiCursor::StateOff(eCursorState stateId)
{
    mNextStateSet.Exclude(stateId);
}

void UiCursor::StatesOff()
{
    mNextStateSet = {eCursorState_Hidden};
}

bool UiCursor::IsOneShotAnimation(eCursorState stateId) const
{
    return mOneShotAnimationSet.Contains(stateId);
}

bool UiCursor::CanChangeState(eCursorState stateId) const
{
    bool isOneShotAnimationPlaying = (IsOneShotAnimation(mCurrentStateId) && !mAnimCycleDone);
    if (!isOneShotAnimationPlaying)
        return true;

    return (stateId <= mCurrentStateId);
}

void UiCursor::UpdateState()
{
    if ((mLastStateSet == mNextStateSet) && !mAnimCycleDone)
        return;

    bool stateChanged = SelectNextState();

    // force reset trigger states
    if (mNextStateSet.HasAny(mOneShotAnimationSet))
    {
        mNextStateSet &= ~mOneShotAnimationSet;
    }

    if (stateChanged)
    {
        mLastStateSet = mNextStateSet;
    }
}

