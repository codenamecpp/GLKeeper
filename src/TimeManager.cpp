#include "pch.h"
#include "TimeManager.h"

TimeManager gTimeManager;

bool TimeManager::Initialize()
{
    mRealtimeFrameDelta = 0.0;
    return true;
}

void TimeManager::Deinit()
{
}

void TimeManager::UpdateFrame(double deltaTime)
{
    mRealtimeFrameDelta = deltaTime;
}
