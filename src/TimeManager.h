#pragma once

//////////////////////////////////////////////////////////////////////////

// variable delta clock
enum class eGameClock
{
    Realtime,
    Gametime,
    Ui,
};

// fixed step clock
enum class eFixedClock
{
    GameLogic, // game turns, logic
    GamePhysics,
};

//////////////////////////////////////////////////////////////////////////

struct GameClock
{
public:
    GameClock() = default;

    inline void SetTimeScale(float timeScale) { mTimeScale = timeScale; }
    inline void Reset()
    {
        mFrameDelta = 0.0f;
        mTime       = 0.0f;
        mTimeScale  = 1.0f;
    }
    inline float GetFrameDelta() const { return mFrameDelta; }
    inline float GetTime() const { return mTime; }
    inline float GetTimeScale() const { return mTimeScale; }
public:
    float mFrameDelta = 0.0f;
    float mTime = 0.0f;
    float mTimeScale = 1.0f;
};

//////////////////////////////////////////////////////////////////////////

struct FixedClock
{
public:
    FixedClock() = default;

    inline void SetFramerate(float framesPerSecond)
    {
        cxx_assert(framesPerSecond >= 0.0f); // 0.0 disables clock
        mFramesPerSecond = framesPerSecond;
        mFrameDelta = (framesPerSecond > 0.0f) ? (1.0f / framesPerSecond) : 0.0f;
    }
    inline void Reset()
    {
        mAccumulator = 0.0f;
        mFrameSteps = 0;
    }
    inline float GetFramesPerSecond() const { return mFramesPerSecond; }
    inline float GetFrameDelta() const { return mFrameDelta; }
    inline float GetAccumulator() const { return mAccumulator; }
    inline int GetFrameSteps() const { return mFrameSteps; }
public:
    float mFramesPerSecond = 0.0f;
    float mFrameDelta = 0.0f; // fixed
    float mAccumulator = 0.0f;
    int mFrameSteps = 0; // number of fixed updates THIS frame
};

//////////////////////////////////////////////////////////////////////////

class TimeManager
{
public:
    bool Initialize();
    void Shutdown();

    void EnterWorld();
    void ClearWorld();
    void UpdateFrame();

    void SetMaxFrameDeltaTime(float maxDeltaTime);

    // clocks
    void SetTimeScale(eGameClock clockType, float scaleValue);
    void ResetTimeScale(eGameClock clockType);
    void ResetClock(eGameClock clockType);

    inline const GameClock& GetClock(eGameClock clockType) const
    {
        switch (clockType)
        {
            case eGameClock::Realtime : return mRealtimeClock;
            case eGameClock::Gametime : return mGametimeClock;
            case eGameClock::Ui : return mUiClock;
        }
        // fallback
        cxx_assert_once(false);
        return mRealtimeClock; 
    }

    // fixed clocks
    void SetFixedClockFramerate(eFixedClock clockType, float framesPerSecond);
    void ResetFixedClock(eFixedClock clockType);

    inline const FixedClock& GetFixedClock(eFixedClock clockType) const
    {
        switch (clockType)
        {
            case eFixedClock::GamePhysics: return mPhysicsClock;
            case eFixedClock::GameLogic: return mLogicClock;
        }
        // fallback
        cxx_assert_once(false);
        return mLogicClock; 
    }

    // shortcuts
    float GetFrameDelta(eGameClock clockType) const;
    float GetTime(eGameClock clockType) const;
    float GetTimeScale(eGameClock clockType) const;
    float GetFixedDelta(eFixedClock clockType) const;
    int GetFixedSteps(eFixedClock clockType) const;

    // helpers
    float TicksToSeconds(eFixedClock clockType, int gameTicks) const;

    void SleepForSeconds(float seconds);

private:
    void AdvanceClock(GameClock& gameClock, float deltaTime);
    void AdvanceFixedClock(FixedClock& fixedClock, float deltaTime);
    void ResetClocks();

    inline GameClock& GetClockMutable(eGameClock clockType) 
    {
        switch (clockType)
        {
            case eGameClock::Realtime: return mRealtimeClock;
            case eGameClock::Gametime: return mGametimeClock;
            case eGameClock::Ui: return mUiClock;
        }
        // fallback
        cxx_assert_once(false);
        return mRealtimeClock; 
    }

    inline FixedClock& GetFixedClockMutable(eFixedClock clockType)
    {
        switch (clockType)
        {
            case eFixedClock::GamePhysics: return mPhysicsClock;
            case eFixedClock::GameLogic: return mLogicClock;
        }
        // fallback
        cxx_assert_once(false);
        return mLogicClock; 
    }

private:
    // params
    float mMaxFrameDeltaTime = 0.0f;

    // state
    double mLastFrameTime = 0.0;

    // clocks
    GameClock mRealtimeClock;
    GameClock mGametimeClock;
    GameClock mUiClock;

    // fixed clocks
    FixedClock mLogicClock;
    FixedClock mPhysicsClock;
};

//////////////////////////////////////////////////////////////////////////

extern TimeManager gTime;

//////////////////////////////////////////////////////////////////////////