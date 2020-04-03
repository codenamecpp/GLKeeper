#pragma once

#include "BinaryInputStream.h"

// scenario data loader
class ScenarioLoader
{
public:
    ScenarioLoader(ScenarioData& scenarioData)
        : mScenarioData(scenarioData)
    {
    }

    bool LoadScenarioData(const std::string& scenario);

private:

    enum eLevelDataFile 
    {
        DKLD_GLOBALS            = 0,
        DKLD_MAP                = 100,
        DKLD_TERRAIN            = 110,
        DKLD_ROOMS              = 120,
        DKLD_TRAPS              = 130,
        DKLD_DOORS              = 140,
        DKLD_KEEPER_SPELLS      = 150,
        DKLD_CREATURE_SPELLS    = 160,
        DKLD_CREATURES          = 170,
        DKLD_PLAYERS            = 180,
        DKLD_THINGS             = 190,
        DKLD_TRIGGERS           = 210,
        DKLD_LEVEL              = 220,
        DKLD_VARIABLES          = 230,
        DKLD_OBJECTS            = 240,
        DKLD_EFFECT_ELEMENTS    = 250,
        DKLD_SHOTS              = 260,
        DKLD_EFFECTS            = 270
    };

    struct LevelDataFilePath
    {
    public:
        LevelDataFilePath() {}
        LevelDataFilePath(eLevelDataFile data_typeid, const std::string& filename)
            : mId(data_typeid)
            , mFilePath(filename)
        {
        }
    public:
        eLevelDataFile mId;
        std::string mFilePath;
    };

    struct KwdFileHeader
    {
        unsigned int mFileSize;
        unsigned int mContentSize;
        unsigned int mHeaderEndOffset;
        int mItemsCount;
    };

    bool ReadString(BinaryInputStream* fileStream, unsigned int stringLength, std::string& ansiString);
    bool ReadString8(BinaryInputStream* fileStream, unsigned int stringLength, std::string& ansiString);
    bool ReadTimestamp(BinaryInputStream* fileStream);
    bool Read32bitsFloat(BinaryInputStream* fileStream, float& outputFloat);
    bool ReadLight(BinaryInputStream* fileStream);
    bool ReadArtResource(BinaryInputStream* fileStream, ArtResource& artResource);
    bool ReadTerrainFlags(BinaryInputStream* fileStream, TerrainDefinition& terrainDef);
    bool ReadRoomFlags(BinaryInputStream* fileStream, RoomDefinition& roomDef);
    bool ReadObjectFlags(BinaryInputStream* fileStream, GameObjectDefinition& objectDef);
    bool ReadStringId(BinaryInputStream* fileStream);
    bool ReadMapData(BinaryInputStream* fileStream);
    bool ReadObjectDefinition(BinaryInputStream* fileStream, GameObjectDefinition& objectDef);
    bool ReadObjectsData(BinaryInputStream* fileStream, int numElements);
    bool ReadPlayerDefinition(BinaryInputStream* fileStream, PlayerDefinition& playerDef);
    bool ReadPlayersData(BinaryInputStream* fileStream, int numElements);
    bool ReadRoomDefinition(BinaryInputStream* fileStream, RoomDefinition& roomDef);
    bool ReadRoomsData(BinaryInputStream* fileStream, int numElements);
    bool ReadCreaturesDefinition(BinaryInputStream* fileStream, const KwdFileHeader& header, CreatureDefinition& creature);
    bool ReadCreaturesData(BinaryInputStream* fileStream, const KwdFileHeader& header);
    bool ReadTerrainDefinition(BinaryInputStream* fileStream, TerrainDefinition& terrainDef);
    bool ReadTerrainData(BinaryInputStream* fileStream, int numElements);
    bool ReadLevelVariables(BinaryInputStream* fileStream);
    bool ReadMapInfo(BinaryInputStream* fileStream);
    bool ReadDataFile(BinaryInputStream* fileStream, eLevelDataFile dataTypeId);
    bool ScanTerrainTypes();
    void FixTerrainResources();

private:
    std::vector<LevelDataFilePath> mPaths;
    ScenarioData& mScenarioData;
};