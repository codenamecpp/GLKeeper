#pragma once

//////////////////////////////////////////////////////////////////////////

#include "ScenarioDefs.h"

//////////////////////////////////////////////////////////////////////////

class DK2ScenarioReader: public cxx::noncopyable
{
private:

    //////////////////////////////////////////////////////////////////////////
    struct LevelDataFilePath
    {
    public:
        LevelDataFilePath() = default;
        LevelDataFilePath(unsigned int dataTypeId, const std::string& fileName)
            : nDataTypeId(dataTypeId)
            , mFilePath(fileName)
        {}
    public:
        unsigned int nDataTypeId;
        std::string mFilePath;
    };

    struct KWDFileHeader
    {
    public:
        unsigned int mTypeId;
        unsigned int mFileSize;
        unsigned int mContentSize;
        unsigned int mHeaderEndOffset;
        int mItemsCount;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    DK2ScenarioReader() = default;

    bool ReadScenarioData(const std::string& filePath, ScenarioDefinition& scenarioData);

private:
    void ApplyExtensions(JsonDocument& extensionsData, ScenarioDefinition& scenarioData) const;
    bool ExploreTerrainTypes(ScenarioDefinition& scenarioData) const;
    bool ReadDataFile(KWDFileHeader& fileHeader, ScenarioDefinition& scenarioData);
    bool ReadDataFile(ScenarioDefinition& scenarioData);
    bool ReadMapInfo(ScenarioDefinition& scenarioData, std::vector<LevelDataFilePath>& paths);
    bool ReadLevelVariables(ScenarioDefinition& scenarioData);
    bool ReadTerrainData(int numElements, ScenarioDefinition& scenarioData);
    bool ReadTerrainDefinition(TerrainDefinition& terrainDef);
    bool ReadCreatureFlags(CreatureDefinition& creatureDef);
    bool ReadCreatureJobAlternative();
    bool ReadCreatureJobPreference();
    bool ReadCreatureResistance();
    bool ReadCreatureSpell();
    bool ReadCreatureAttraction();
    bool ReadCreaturesData(const KWDFileHeader& header, ScenarioDefinition& scenarioData);
    bool ReadCreaturesDefinition(CreatureDefinition& creature);
    bool ReadRoomsData(int numElements, ScenarioDefinition& scenarioData);
    bool ReadRoomDefinition(RoomDefinition& roomDef);
    bool ReadPlayersData(int numElements, ScenarioDefinition& scenarioData);
    bool ReadPlayerDefinition(PlayerDefinition& playerDef);
    bool ReadCreatureThingBehaviorFlags(ScenarioCreatureThing& creatureThingData);
    bool ReadCreatureThingData(ScenarioCreatureThing& creatureThingData);
    bool ReadObjectThingData(ScenarioObjectThing& objectThingData);
    bool ReadRoomThingData(ScenarioRoomThing& roomThingData);
    bool ReadThingsData(int numElements, ScenarioDefinition& scenarioData);
    bool ReadObjectsData(int numElements, ScenarioDefinition& scenarioData);
    bool ReadObjectDefinition(GameObjectDefinition& objectDef);
    bool ReadMapData(ScenarioDefinition& scenarioData);
    bool ReadScenarioVariables(int numElements, ScenarioDefinition& scenarioData);
    bool ReadStringId();
    bool ReadObjectFlags(GameObjectDefinition& objectDef);
    bool ReadRoomFlags(RoomDefinition& roomDef);
    bool ReadTerrainFlags(TerrainDefinition& terrainDef);
    bool ReadArtResource(ArtResourceDefinition& artResource);
    bool ReadLight();
    bool Read32bitsFloat(float& outputFloat);
    bool ReadVector3f(glm::vec3& outputVector);
    bool ReadTimestamp();
    bool ReadString8(unsigned int stringLength, std::string& ansiString);
    bool ReadString(unsigned int stringLength, std::wstring& wideString);

private:
    std::ifstream mFileStream;
};

//////////////////////////////////////////////////////////////////////////