#pragma once

//////////////////////////////////////////////////////////////////////////

class SimpleTimer
{
public:
    SimpleTimer() = default;

    explicit SimpleTimer(float timerDuration)
        : mDuration(timerDuration)
    {
    }

    // Configure timer duration without changing its current state
    inline void SetDuration(float timerDuration) { mDuration = timerDuration; }

    // Restart timer with previously configured duration, without preserving the overtime
    inline void Start()
    {
        mElapsedTime = 0.0f;
        mStarted = (mDuration > 0.0f);
    }

    // Restart timer with specified duration, without preserving the overtime
    inline void Start(float timerDuration)
    {
        SetDuration(timerDuration);
        Start();
    }

    inline bool ConsumeOvertime(float overtime)
    {
        if (overtime > 0.0f)
        {
            float currentOvertime = GetOvertime();
            if (currentOvertime > 0.0f)
            {
                mElapsedTime -= std::min(currentOvertime, overtime);
                return true;
            }
        }
        return false;
    }

    // Stop timer, clear its state
    inline void Stop()
    {
        mElapsedTime = 0.0f;
        mStarted = false;
    }

    // Advance timer if it was started
    inline void Tick(float deltaTime)
    {
        if (mStarted)
        {
            mElapsedTime += deltaTime;
        }
    }

    // Advance timer and check if it expired
    inline bool TickAndCheckExpire(float deltaTime)
    {
        Tick(deltaTime);
        return IsExpired();
    }

    // State
    inline bool IsStarted() const { return mStarted; }
    inline bool IsExpired() const { return mStarted && (mElapsedTime >= mDuration); }
    inline bool IsOngoing() const { return mStarted && (mElapsedTime  < mDuration); }

    // Time
    inline float GetDuration() const { return mDuration; }
    inline float GetElapsedTime() const { return mElapsedTime; }
    inline float GetOvertime() const 
    { 
        return (mElapsedTime > mDuration) ? (mElapsedTime - mDuration) : 0.0f; 
    }

    inline operator bool () const { return IsStarted(); }

private:
    float mDuration = 0.0f;
    float mElapsedTime = 0.0f;

    bool mStarted = false;
};

//////////////////////////////////////////////////////////////////////////