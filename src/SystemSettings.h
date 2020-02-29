#pragma once

#include "CommonTypes.h"

// system settings from file
struct SystemSettings
{
public:
    Size2D mScreenDimensions;
    bool mFullscreen = false; // enable fullscreen mode
    bool mEnableVSync = false; // enable vertical synchronization

public:
    SystemSettings() = default;

    void SetDefaults();
};

//////////////////////////////////////////////////////////////////////////

// system startup params from command line
struct SystemStartupParams
{
public:
    // load system config from custom file instead of the default
    std::string mCustomConfigFileName;

    // full path to dungeon keeper game folder
    std::string mDungeonKeeperGamePath;

public:
    SystemStartupParams() = default;

    bool ParseStartupParams(int argc, char *argv[]);
    void Clear();
};