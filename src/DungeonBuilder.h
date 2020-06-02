#pragma once

// helper class for map mesh construction
class DungeonBuilder
{
public:
    // rebuild terrain mesh for specific tile
    void BuildTerrainMesh(TerrainTile* mapTile);
    void BuildTerrainMesh(TerrainTile* mapTile, eTileFace faceid);

    // append geometry to specific tile face mesh
    // @param mapTile: Map tile
    // @param faceid: Tile face identifier
    // @param asset: Geometry asset
    // @param rot: Optional rotation matrix
    // @param trans: Optional translation matrix
    void ExtendTileMesh(TerrainTile* mapTile, eTileFace faceid, ModelAsset* asset, const glm::mat3* rot = 0, const glm::vec3* trans = 0);

    // helpers
    bool NeighbourTileSolid(TerrainTile* mapTile, eDirection direction) const;
    bool NeighbourHasSameTerrain(TerrainTile* mapTile, eDirection direction) const;
    bool NeighbourHasSameBaseTerrain(TerrainTile* mapTile, eDirection direction) const;

    // test whether
    // - Tile is Solid and Allows room walls
    // - Terrain type of neighbour tile is Room and it has Walls
    // So geometry for this wall should be managed by room
    bool NeighbourRoomHandlesWalls(TerrainTile* mapTile, eDirection direction) const;
    bool RoomHandlesWalls(TerrainTile* mapTile) const;

    // test whether neighbour tile has same room instance constructed on it
    bool NeighbourHasSameRoom(TerrainTile* mapTile, eDirection direction) const;

private:
    // construction
    void ConstructTerrainWalls(TerrainTile* mapTile);
    void ConstructTerrainWall(TerrainTile* mapTile, eTileFace faceid);
    void ConstructTerrainFloor(TerrainTile* mapTile);
    void ConstructTerrainQuad(TerrainTile* mapTile, ArtResource* artResource);
    void ConstructTerrainWaterBed(TerrainTile* mapTile, ArtResource* artResource);

    // test whether side wall should be constructed
    // @param dungeonMapTile: Target map tile
    // @param direction: Wall side
    bool ShouldBuildSideWall(TerrainTile* mapTile, eTileFace faceid) const;
};