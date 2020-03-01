#pragma once

#include "SystemSettings.h"

// this class provides common system functionality
class System: public cxx::noncopyable
{
public:
    // readonly
    SystemSettings mSettings;
    SystemStartupParams mStartupParams;

public:

    // setup system internal resources, terminates application on error
    void Initialize(int argc, char *argv[]);
    void Deinit();

    // initialize application subsystems and run main loop
    void Execute();

    // abnormal application shutdown due to critical failure
    void Terminate();

    // set application exit request flag, execution will be interrupted on next frame
    void QuitRequest();

    // get real time seconds since system started
    double GetSysTime() const;

private:
    // save/load settings to/from external file
    void SaveSettings();
    void LoadSettings();

private:
    double mStartSystemTime = 0.0;
    bool mQuitRequested = false;
};

extern System gSystem;