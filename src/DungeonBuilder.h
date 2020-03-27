#pragma once

#include "GameDefs.h"
#include "ScenarioDefs.h"

// helper class for map mesh construction
class DungeonBuilder
{
public:
    // rebuild terrain mesh for specific tile
    void BuildTerrainMesh(MapTile* mapTile);
    void BuildTerrainMesh(MapTile* mapTile, eTileFace faceid);

    // append geometry to specific tile face mesh
    // @param mapTile: Map tile
    // @param faceid: Tile face identifier
    // @param asset: Geometry asset
    // @param rot: Optional rotation matrix
    // @param trans: Optional translation matrix
    void ExtendTileMesh(MapTile* mapTile, eTileFace faceid, ModelAsset* asset, const glm::mat3* rot = 0, const glm::vec3* trans = 0);

    // helpers
    bool NeighbourTileSolid(MapTile* mapTile, eDirection direction) const;
    bool NeighbourHasSameTerrain(MapTile* mapTile, eDirection direction) const;
    bool NeighbourHasSameBaseTerrain(MapTile* mapTile, eDirection direction) const;

    // test whether
    // - Tile is Solid and Allows room walls
    // - Terrain type of neighbour tile is Room and it has Walls
    // So geometry for this wall should be managed by room
    bool NeighbourRoomHandlesWalls(MapTile* mapTile, eDirection direction) const;
    bool RoomHandlesWalls(MapTile* mapTile) const;

    // test whether neighbour tile has same room instance constructed on it
    bool NeighbourHasSameRoom(MapTile* mapTile, eDirection direction) const;

private:
    // construction
    void ConstructTerrainWalls(MapTile* mapTile);
    void ConstructTerrainWall(MapTile* mapTile, eTileFace faceid);
    void ConstructTerrainFloor(MapTile* mapTile);
    void ConstructTerrainQuad(MapTile* mapTile, ArtResource* artResource);
    void ConstructTerrainWaterBed(MapTile* mapTile, ArtResource* artResource);

    // test whether side wall should be constructed
    // @param dungeonMapTile: Target map tile
    // @param direction: Wall side
    bool ShouldBuildSideWall(MapTile* mapTile, eTileFace faceid) const;
};