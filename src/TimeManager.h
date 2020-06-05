#pragma once

class TimeManager: public cxx::noncopyable
{
public:
    // setup manager internal resources
    bool Initialize();
    void Deinit();

    // process single frame logic
    // @param deltaTime: Time elapsed since last frame in seconds, realtime
    void UpdateFrame(double deltaTime);

    // get time elapsed since last frame specified in seconds, realtime
    inline double GetRealtimeFrameDelta() const { return mRealtimeFrameDelta; }

private:
    double mRealtimeFrameDelta;
};

extern TimeManager gTimeManager;