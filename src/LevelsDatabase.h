#pragma once

//////////////////////////////////////////////////////////////////////////

#include "ScenarioDefs.h"

//////////////////////////////////////////////////////////////////////////

class LevelsDatabase: public cxx::noncopyable
{
public:

    bool Initialize();
    void Shutdown();

    bool HasLevelInfo(const std::string& fileName) const;
    bool GetLevelInfo(const std::string& fileName, ScenarioLevelInfo& levelInfo) const;

    template<typename EnumProc>
    inline void EnumMyPetDungeonLevels(EnumProc enumProc) const
    {
        for (const ScenarioLevelInfo& roller: mLevelInfoList)
        {
            if (roller.mFlags.mIsMyPetDungeonLevel)
            {
                enumProc(roller);
            }
        }
    }

    template<typename EnumProc>
    inline void EnumSkirmishLevels(EnumProc enumProc) const
    {
        for (const ScenarioLevelInfo& roller: mLevelInfoList)
        {
            if (roller.mFlags.mIsSkirmishLevel)
            {
                enumProc(roller);
            }
        }
    }

    template<typename EnumProc>
    inline void EnumSecretLevels(EnumProc enumProc) const
    {
        for (const ScenarioLevelInfo& roller: mLevelInfoList)
        {
            if (roller.mFlags.mIsSecretLevel)
            {
                enumProc(roller);
            }
        }
    }

    template<typename EnumProc>
    inline void EnumSpecialLevels(EnumProc enumProc) const
    {
        for (const ScenarioLevelInfo& roller: mLevelInfoList)
        {
            if (roller.mFlags.mIsSpecialLevel)
            {
                enumProc(roller);
            }
        }
    }


    template<typename EnumProc>
    inline void EnumMultiplayerLevels(EnumProc enumProc) const
    {
        for (const ScenarioLevelInfo& roller: mLevelInfoList)
        {
            if (roller.mFlags.mIsMultiplayerLevel)
            {
                enumProc(roller);
            }
        }
    }

    // accessing all known levels info
    const auto& GetAllLevels() const { return mLevelInfoList; }

private:
    void HandleLevel(ScenarioLevelInfo&& levelInfo);

private:
    std::vector<ScenarioLevelInfo> mLevelInfoList;
};

//////////////////////////////////////////////////////////////////////////

extern LevelsDatabase gLevelsDatabase;

//////////////////////////////////////////////////////////////////////////