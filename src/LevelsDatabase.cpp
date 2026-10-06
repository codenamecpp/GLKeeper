#include "stdafx.h"
#include "LevelsDatabase.h"
#include "DK2ScenarioReader.h"

//////////////////////////////////////////////////////////////////////////

LevelsDatabase gLevelsDatabase;

//////////////////////////////////////////////////////////////////////////

bool LevelsDatabase::Initialize()
{
    gConsole.LogMessage(eLogLevel_Info, "Initialize levels database...");

    mCampaignLevelKeys =
    {
        // main campaign
        {eCampaignLevelType_Main, 1},
        {eCampaignLevelType_Main, 2},
        {eCampaignLevelType_Main, 3},
        {eCampaignLevelType_Main, 4},
        {eCampaignLevelType_Main, 5},
        {eCampaignLevelType_Main, 6, 'a'},
        {eCampaignLevelType_Main, 6, 'b'},
        {eCampaignLevelType_Main, 7},
        {eCampaignLevelType_Main, 8},
        {eCampaignLevelType_Main, 9},
        {eCampaignLevelType_Main, 10},
        {eCampaignLevelType_Main, 11, 'a'},
        {eCampaignLevelType_Main, 11, 'b'},
        {eCampaignLevelType_Main, 11, 'c'},
        {eCampaignLevelType_Main, 12},
        {eCampaignLevelType_Main, 13},
        {eCampaignLevelType_Main, 14},
        {eCampaignLevelType_Main, 15, 'a'},
        {eCampaignLevelType_Main, 15, 'b'},
        {eCampaignLevelType_Main, 16},
        {eCampaignLevelType_Main, 17},
        {eCampaignLevelType_Main, 18},
        {eCampaignLevelType_Main, 19},
        {eCampaignLevelType_Main, 20},

        // my pet dungeon
        {eCampaignLevelType_MyPetDungeon, 1},
        {eCampaignLevelType_MyPetDungeon, 2},
        {eCampaignLevelType_MyPetDungeon, 3},
        {eCampaignLevelType_MyPetDungeon, 4},
        {eCampaignLevelType_MyPetDungeon, 5},
        {eCampaignLevelType_MyPetDungeon, 6},
        {eCampaignLevelType_MyPetDungeon, 7},

        // secret
        {eCampaignLevelType_Secret, 1},
        {eCampaignLevelType_Secret, 2},
        {eCampaignLevelType_Secret, 3},
        {eCampaignLevelType_Secret, 4},
        {eCampaignLevelType_Secret, 5},
    };

    mLevelInfoList.reserve(128);

    std::string filePath;
    gFiles.EnumMapFiles([&, this](const std::string& fileName)
        {
            if (!gFiles.LocateMapData(fileName, filePath))
            {
                cxx_assert(false);
                return;
            }
            ScenarioLevelInfo levelInfo;
            DK2ScenarioReader scenarioReader;
            if (!scenarioReader.ReadScenarioShortInfo(filePath, levelInfo))
            {
                cxx_assert(false);
                return;
            }
            this->HandleLevel(std::move(levelInfo));
        });

    return !mLevelInfoList.empty();
}

void LevelsDatabase::Shutdown()
{
    mLevelInfoList.clear();
    mCampaignLevelKeys.clear();
}

bool LevelsDatabase::HasLevelInfo(const std::string& fileName) const
{
    return cxx::contains_if(mLevelInfoList, [&fileName](const ScenarioLevelInfo& roller) 
        { 
            return cxx::strings_eq_icase(fileName, roller.mFileName); 
        });
}

bool LevelsDatabase::GetLevelInfo(const std::string& fileName, ScenarioLevelInfo& levelInfo) const
{
    for (const auto& roller: mLevelInfoList)
    {
        if (cxx::strings_eq_icase(fileName, roller.mFileName))
        {
            levelInfo = roller;
            return true;
        }
    }
    return false;
}

bool LevelsDatabase::GetCampaignLevelInfo(const CampaignLevelKey& levelKey, ScenarioLevelInfo& levelInfo) const
{
    bool isSuccess = false;
    if (levelKey)
    {
        std::string fileName;
        switch (levelKey.mType)
        {
            case eCampaignLevelType_Main: fileName = "Level"; break;
            case eCampaignLevelType_Secret: fileName = "Secret"; break;
            case eCampaignLevelType_MyPetDungeon: fileName = "mpd"; break;
        }
        fileName.append(std::to_string(levelKey.mNumber));
        if (levelKey.mVariant)
        {   
            cxx_assert(levelKey.mType == eCampaignLevelType_Main);
            fileName.push_back(levelKey.mVariant);
        }

        isSuccess = GetLevelInfo(fileName, levelInfo);
    }
    return isSuccess;
}

void LevelsDatabase::HandleLevel(ScenarioLevelInfo&& levelInfo)
{
    mLevelInfoList.push_back(levelInfo);
}
