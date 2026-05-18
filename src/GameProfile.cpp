#include "stdafx.h"
#include "GameProfile.h"

//////////////////////////////////////////////////////////////////////////

GameProfile gGameProfile;

//////////////////////////////////////////////////////////////////////////

static const std::string GameProfileFilePath = "config/app_settings.json";

//////////////////////////////////////////////////////////////////////////

bool GameProfile::Initialize()
{
    LoadDefaults();
    LoadGameProfile();
    return true;
}

void GameProfile::Shutdown()
{

}

bool GameProfile::LoadGameProfile(const std::string& filepath)
{
    std::string fullpath;
    JsonDocument jsonSettings;
    if (!gFiles.PathToFile(filepath, fullpath) || !FSLoadJSON(fullpath, jsonSettings))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load game profile data '%s'", filepath.c_str());
        return false;
    }
    // user settings
    JsonQuery(jsonSettings.GetRootElement(), "screen.resolution", mUserSettings.mScreenResolution);
    JsonQuery(jsonSettings.GetRootElement(), "screen.fullscreen", mUserSettings.mEnableFullscreen);
    JsonQuery(jsonSettings.GetRootElement(), "screen.vsync", mUserSettings.mEnableVSync);
    JsonQuery(jsonSettings.GetRootElement(), "hardware_cursor", mUserSettings.mEnableHwCursor);
    JsonQuery(jsonSettings.GetRootElement(), "game_root", mUserSettings.mGameRootFolder);
    // user progress

    return true;
}

bool GameProfile::LoadGameProfile()
{
    if (LoadGameProfile(GameProfileFilePath))
    {
        // dungeon keeper 2 data files location
        if (!mUserSettings.mGameRootFolder.empty())
        {
            gFiles.AddSearchPlace(mUserSettings.mGameRootFolder);
        }
        gConsole.LogMessage(eLogLevel_Info, "Dungeon Keeper II folder is '%s'", mUserSettings.mGameRootFolder.c_str());
        return true;
    }
    return true;
}

bool GameProfile::SaveGameProfile(const std::string& filepath) const
{
    // todo
    return true;
}

bool GameProfile::SaveGameProfile()
{
    return SaveGameProfile(GameProfileFilePath);
}

void GameProfile::PostChangeUserSettings()
{

}

void GameProfile::PostChangeUserProgress()
{

}

void GameProfile::LoadDefaults()
{
    // default video settings
    mUserSettings.mGameRootFolder.clear();
    mUserSettings.mScreenResolution = {1280, 900};
    mUserSettings.mEnableFullscreen = false;
    mUserSettings.mEnableVSync = true;
    mUserSettings.mEnableHwCursor = false;

    // default inputs

    // default sounds
}
