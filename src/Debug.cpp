#include "stdafx.h"
#include "Debug.h"

DebugEngine gDebug;

DebugEngine::DebugEngine()
    : mDrawUiBounds()
    , mFPSCounterMin(1000)
    , mFPSCounterMax()
    , mDrawSceneAabbTree()
    , mNoDrawWaterLava()
    , mNoDrawSceneObjects()
    , mDrawPhysics()
{
}

void DebugEngine::Initialize()
{
    mFPSDeltaTimeAccumulator = 0.0f;
    mFPSCounter = 0;
    mFPSCounterLast = 0;
    mFPSCounterMin = 1000;
    mFPSCounterMax = 0;
}

void DebugEngine::Shutdown()
{
}

void DebugEngine::UpdateFrame()
{
    // process fps
    mFPSDeltaTimeAccumulator += gTime.GetFrameDelta(eGameClock::Realtime);
    ++mFPSCounter;
    if (mFPSDeltaTimeAccumulator >= 1.0f)
    {
        mFPSDeltaTimeAccumulator -= 1.0f;
        mFPSCounterLast = mFPSCounter;
        if (mFPSCounterLast > mFPSCounterMax)
            mFPSCounterMax = mFPSCounterLast;

        if (mFPSCounterLast < mFPSCounterMin)
            mFPSCounterMin = mFPSCounterLast;

        mFPSCounter = 0;
    }
}