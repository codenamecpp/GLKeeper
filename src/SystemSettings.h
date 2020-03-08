#pragma once

#include "CommonTypes.h"

#define SYSTEM_SETTINGS_FILE "config/system_settings.json"

// system settings from file
struct SystemSettings
{
public:
    Size2D mScreenDimensions;
    bool mFullscreen = false; // enable fullscreen mode
    bool mEnableVSync = false; // enable vertical synchronization

    float mScreenAspectRatio = 1.0f;

public:
    SystemSettings() = default;

    void SetDefaults();
    void SetFromJsonDocument(const cxx::json_document& sourceDocument);
    void StoreToJsonDocument(cxx::json_document& sourceDocument);
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