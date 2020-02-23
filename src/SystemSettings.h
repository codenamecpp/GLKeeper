#pragma once

// system settings from file
struct SystemSettings
{
public:
    // graphics
    int mScreenSizex = 0; // screen dimensions
    int mScreenSizey = 0; 
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