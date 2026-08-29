#include "stdafx.h"
#include "LevelsDatabase.h"
#include "DK2ScenarioReader.h"

//////////////////////////////////////////////////////////////////////////

LevelsDatabase gLevelsDatabase;

//////////////////////////////////////////////////////////////////////////

bool LevelsDatabase::Initialize()
{
    gConsole.LogMessage(eLogLevel_Info, "Initialize levels database...");

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

void LevelsDatabase::HandleLevel(ScenarioLevelInfo&& levelInfo)
{
    mLevelInfoList.push_back(levelInfo);
}
