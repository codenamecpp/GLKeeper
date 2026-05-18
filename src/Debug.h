#pragma once

//////////////////////////////////////////////////////////////////////////
// DebugEngine
//////////////////////////////////////////////////////////////////////////

class DebugEngine
{
public:
    DebugEngine();

    void Initialize();
    void Shutdown();
    void UpdateFrame();

public:
    bool mDrawUiBounds;
    bool mDrawSceneAabbTree;
    bool mNoDrawSceneObjects;
    bool mDrawPhysics;
    bool mNoDrawWaterLava;

    // fps counting
    float mFPSDeltaTimeAccumulator = 0.0f;
    int mFPSCounter;
    int mFPSCounterLast;
    int mFPSCounterMin;
    int mFPSCounterMax;
};

extern DebugEngine gDebug;