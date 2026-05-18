#pragma once

// GameProfile stores, saves and loads user settings and progress data

class GameProfile
{
public:

    //////////////////////////////////////////////////////////////////////////

    struct UserSettings
    {
    public:
        UserSettings() = default;

    public:
        // common
        std::string mGameRootFolder;
        // graphics
        Point2D mScreenResolution;
        bool mEnableVSync = true;
        bool mEnableFullscreen = false;
        bool mEnableHwCursor = false;
        // inputs
        // sounds
    };

    //////////////////////////////////////////////////////////////////////////

    struct UserProgress
    {
    public:
    };

    //////////////////////////////////////////////////////////////////////////

public:
    bool Initialize();
    void Shutdown();

    bool LoadGameProfile();
    bool SaveGameProfile();

    template<typename TChangeProc>
    inline void ChangeUserSettings(TChangeProc changeProc)
    {
        changeProc(mUserSettings);
        PostChangeUserSettings();
    }

    template<typename TChangeProc>
    inline void ChangeUserProgress(TChangeProc changeProc)
    {
        changeProc(mUserProgress);
        PostChangeUserProgress();
    }

    const UserSettings& GetUserSettings() const { return mUserSettings; }
    const UserProgress& GetUserProgress() const { return mUserProgress; }

private:
    void LoadDefaults();
    bool LoadGameProfile(const std::string& filepath);
    bool SaveGameProfile(const std::string& filepath) const;
    void PostChangeUserSettings();
    void PostChangeUserProgress();

private:
    UserSettings mUserSettings;
    UserProgress mUserProgress;
};

extern GameProfile gGameProfile;