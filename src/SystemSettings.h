#pragma once

#define SYSTEM_SETTINGS_FILE "config/system_settings.json"

// system settings from file
struct SystemSettings
{
public:
    Point mScreenDimensions;
    bool mFullscreen = false; // enable fullscreen mode
    bool mResizeableWindow = false; // enable resize in windowed mode
    bool mEnableVSync = false; // enable vertical synchronization
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

    // custom map name to load
    std::string mStartupMapName;

public:
    SystemStartupParams() = default;

    bool ParseStartupParams(int argc, char *argv[]);
    void Clear();
};