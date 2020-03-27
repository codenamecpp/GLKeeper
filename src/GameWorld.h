#pragma once

#include "ScenarioDefs.h"
#include "GameMap.h"
#include "DungeonBuilder.h"

// game world state
class GameWorld: public cxx::noncopyable
{
public:
    // readonly
    ScenarioData mScenarioData;
    GameMap mMapData;
    DungeonBuilder mDungeonBuilder;

public:
    
    // load world data, level map, setup players, build rooms etc
    // @param scenarioName: Scenario name
    bool InitializeWorld(const std::string& scenarioName);
    void Deinit();

    // process single frame logic
    void UpdateFrame();

    // get terrain definition by type name or type identifier
    // @param typeName: Type name
    // @param typeID: Type identifier
    TerrainDefinition* GetTerrainDefinition(const std::string& typeName);
    TerrainDefinition* GetTerrainDefinition(TerrainTypeID typeID);

    // well known terrain types
    TerrainDefinition* GetLavaTerrain();
    TerrainDefinition* GetWaterTerrain();
    TerrainDefinition* GetPlayerColouredPathTerrain();
    TerrainDefinition* GetPlayerColouredWallTerrain();
    TerrainDefinition* GetFogOfWarTerrain();

    // get game object definition by type name or type identifier
    // @param typeName: Type name
    // @param typeID: Type identifier
    GameObjectDefinition* GetGameObjectDefinition(const std::string& typeName);
    GameObjectDefinition* GetGameObjectDefinition(GameObjectTypeID typeID);

    // get room definition by type name or type identifier
    // @param typeName: Type name
    // @param typeID: Type identifier
    RoomDefinition* GetRoomDefinition(const std::string& typeName);
    RoomDefinition* GetRoomDefinition(RoomTypeID typeID);

    // get creature definition by type name or type identifier
    // @param typeName: Type name
    // @param typeID: Type identifier
    CreatureDefinition* GetCreatureDefinition(const std::string& typeName);
    CreatureDefinition* GetCreatureDefinition(CreatureTypeID typeID);

    // get room definition by corresponding terrain identifier
    RoomDefinition* GetRoomDefinitionByTerrain(TerrainDefinition* terrainDefinition);
    RoomDefinition* GetRoomDefinitionByTerrain(TerrainTypeID typeID);

    // test whether terrain is room
    bool IsRoomTypeTerrain(TerrainDefinition* terrainDefinition) const;
    bool IsRoomTypeTerrain(TerrainTypeID typeID) const;

private:
    void SetupMapData(unsigned int seed);
    void ConstructStartupRooms();
    void ConstructStartupRoom(MapTile* initialTile);
};

extern GameWorld gGameWorld;