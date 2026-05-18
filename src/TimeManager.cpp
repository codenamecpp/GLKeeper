#include "stdafx.h"
#include "TimeManager.h"

//////////////////////////////////////////////////////////////////////////
TimeManager gTime;
//////////////////////////////////////////////////////////////////////////

bool TimeManager::Initialize()
{
    mLastFrameTime = 0.0;

    SetMaxFrameDeltaTime(1.0 / 8.0f); // min framerate 

    // init fixed clock
    SetFixedClockFramerate(eFixedClock::GameLogic, 4.0f); // base tick rate
    SetFixedClockFramerate(eFixedClock::GamePhysics, 10.0f);

    ResetClocks();

    return true;
}

void TimeManager::Shutdown()
{

}

void TimeManager::EnterWorld()
{

}

void TimeManager::ClearWorld()
{

}

void TimeManager::UpdateFrame()
{
    double currentFrameTime = ::glfwGetTime();

    float deltaTime = static_cast<float>(currentFrameTime - mLastFrameTime);
    mLastFrameTime = currentFrameTime;
    // cap frame delta
    if (deltaTime > mMaxFrameDeltaTime) 
    {
        deltaTime = mMaxFrameDeltaTime;
    }

    // advance realtime clock
    AdvanceClock(mRealtimeClock, deltaTime);
    // advance game clocks, delta source is realtime clock
    AdvanceClock(mGametimeClock, mRealtimeClock.mFrameDelta);
    AdvanceClock(mUiClock, mRealtimeClock.mFrameDelta);
    // advance fixed clocks, delta source is gametick clock
    AdvanceFixedClock(mLogicClock, mGametimeClock.mFrameDelta);
    AdvanceFixedClock(mPhysicsClock, mGametimeClock.mFrameDelta);

    // smallsleep
    ::glfwWaitEventsTimeout(0.001);
}

void TimeManager::SetMaxFrameDeltaTime(float maxDeltaTime)
{
    cxx_assert(maxDeltaTime >= 0.0f);

    mMaxFrameDeltaTime = maxDeltaTime;
}

void TimeManager::SetTimeScale(eGameClock clockType, float scaleValue)
{
    cxx_assert(scaleValue >= 0.0f);
    cxx_assert(clockType != eGameClock::Realtime);

    if (clockType != eGameClock::Realtime)
    {
        GameClock& gameClock = GetClockMutable(clockType);
        gameClock.mTimeScale = std::max(scaleValue, 0.0f);
    }
}

void TimeManager::ResetTimeScale(eGameClock clockType)
{
    if (clockType != eGameClock::Realtime)
    {
        SetTimeScale(clockType, 1.0f);
    }
}

void TimeManager::ResetClock(eGameClock clockType)
{
    cxx_assert(clockType != eGameClock::Realtime);

    if (clockType != eGameClock::Realtime)
    {
        GameClock& gameClock = GetClockMutable(clockType);
        gameClock.Reset();
    }
}

void TimeManager::SetFixedClockFramerate(eFixedClock clockType, float framesPerSecond)
{
    cxx_assert(framesPerSecond >= 0.0f);

    FixedClock& fixedClock = GetFixedClockMutable(clockType);
    fixedClock.SetFramerate(framesPerSecond);
}

void TimeManager::ResetFixedClock(eFixedClock clockType)
{
    FixedClock& fixedClock = GetFixedClockMutable(clockType);
    fixedClock.Reset();
}

void TimeManager::ResetClocks()
{
    mRealtimeClock.Reset();
    mGametimeClock.Reset();
    mUiClock.Reset();
    mLogicClock.Reset();
    mPhysicsClock.Reset();
}

void TimeManager::AdvanceClock(GameClock& gameClock, float deltaTime)
{
    if (gameClock.mTimeScale > 0.0f)
    {
        gameClock.mFrameDelta = (deltaTime * gameClock.mTimeScale);
        gameClock.mTime += gameClock.mFrameDelta;
    }
}

void TimeManager::AdvanceFixedClock(FixedClock& fixedClock, float deltaTime)
{
    fixedClock.mFrameSteps = 0;

    if ((deltaTime > 0.0f) && (fixedClock.mFrameDelta > 0.0f))
    {
        fixedClock.mAccumulator += deltaTime;
        while (fixedClock.mAccumulator >= fixedClock.mFrameDelta)
        {
            fixedClock.mAccumulator -= fixedClock.mFrameDelta;
            fixedClock.mFrameSteps++;
        }
    }
}

float TimeManager::GetFrameDelta(eGameClock clockType) const 
{ 
    return GetClock(clockType).GetFrameDelta(); 
}

float TimeManager::GetTime(eGameClock clockType) const 
{ 
    return GetClock(clockType).GetTime(); 
}

float TimeManager::GetTimeScale(eGameClock clockType) const 
{ 
    return GetClock(clockType).GetTimeScale(); 
}

float TimeManager::GetFixedDelta(eFixedClock clockType) const 
{ 
    return GetFixedClock(clockType).GetFrameDelta(); 
}

int TimeManager::GetFixedSteps(eFixedClock clockType) const 
{ 
    return GetFixedClock(clockType).GetFrameSteps(); 
}

float TimeManager::TicksToSeconds(eFixedClock clockType, int gameTicks) const
{
    return GetFixedClock(clockType).GetFrameDelta() * (gameTicks * 1.0f);
}

void TimeManager::SleepForSeconds(float seconds)
{
    std::this_thread::sleep_for(std::chrono::duration<float>(seconds));
}
