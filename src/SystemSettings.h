#pragma once

// system settings from file
struct SystemSettings
{
public:
    // todo
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

    bool ParseStartupParams(int argc, char *argv[]);
    void Clear();
   
};