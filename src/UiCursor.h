#pragma once

//////////////////////////////////////////////////////////////////////////

#include "AssetDefs.h"
#include "UiDefs.h"
#include "SimpleTimer.h"

//////////////////////////////////////////////////////////////////////////

class UiCursor final: public cxx::noncopyable
{
public:
    //////////////////////////////////////////////////////////////////////////

    // states ordered from highest to lowest priority

    enum eCursorState : unsigned short
    {
        eCursorState_Hidden, // <- highest priority
        eCursorState_PickAxeTag,
        eCursorState_PointOnUi,
        eCursorState_Slap,
        eCursorState_PickAxeHold,
        eCursorState_DropThing,
        eCursorState_DropGold,
        eCursorState_SpellCast,
        eCursorState_PointOnThing,
        eCursorState_HoldThing,
        eCursorState_HoldGold,
        eCursorState_SpellHold,
        eCursorState_Idle,  // <- lowest priority
        // 
        eCursorState_COUNT
    };
    //////////////////////////////////////////////////////////////////////////
private:
    //////////////////////////////////////////////////////////////////////////
    struct CursorState
    {
    public:
        Texture* mTexture {};
        std::vector<Rect2D> mAnimationFrames;
        Point2D mHotSpot; // common to all animation frames
    };
    //////////////////////////////////////////////////////////////////////////

public:

    void Init();
    void Deinit();

    void RenderFrame(UiRenderContext& renderContext);
    void UpdateFrame(float deltaTime);

    void StateOn(eCursorState stateId);
    void StateOff(eCursorState stateId);
    void StatesOff();

private:
    void InitCursorState(eCursorState stateId, const std::string& textureName, const Point2D& hotSpot, int frameCount);
    void UpdateStateAnimation(float deltaTime);
    void UpdateState();
    bool IsOneShotAnimation(eCursorState stateId) const;

    bool SelectNextState();
    bool CanChangeState(eCursorState stateId) const;
    void ChangeState(eCursorState stateId);

    eCursorState DetectNextState() const;

private:
    using CursorStateSet = EnumSet<eCursorState>;

    CursorState     mStates[eCursorState_COUNT];
    Point2D         mMousePosition {};

    eCursorState    mCurrentStateId = eCursorState_Hidden;

    CursorStateSet  mLastStateSet {};
    CursorStateSet  mNextStateSet {};
    CursorStateSet  mOneShotAnimationSet {};

    // animation state
    SimpleTimer     mAnimFrameTimer;
    int             mAnimFrameIndex {};
    bool            mAnimCycleDone {};
};

//////////////////////////////////////////////////////////////////////////

extern UiCursor gUiCursor;

//////////////////////////////////////////////////////////////////////////