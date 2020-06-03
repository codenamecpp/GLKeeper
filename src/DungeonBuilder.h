#pragma once

// helper class for map mesh construction
class DungeonBuilder
{
public:
    // rebuild terrain mesh for specific tile
    void BuildTerrainMesh(TerrainTile* terrainTile);
    void BuildTerrainMesh(TerrainTile* terrainTile, eTileFace faceid);

    // append geometry to specific tile face mesh
    // @param terrainTile: Map tile
    // @param faceid: Tile face identifier
    // @param asset: Geometry asset
    // @param rot: Optional rotation matrix
    // @param trans: Optional translation matrix
    void ExtendTileMesh(TerrainTile* terrainTile, eTileFace faceid, ModelAsset* asset, const glm::mat3* rot = 0, const glm::vec3* trans = 0);

    // helpers
    bool NeighbourTileSolid(TerrainTile* terrainTile, eDirection direction) const;
    bool NeighbourHasSameTerrain(TerrainTile* terrainTile, eDirection direction) const;
    bool NeighbourHasSameBaseTerrain(TerrainTile* terrainTile, eDirection direction) const;

    // test whether
    // - Tile is Solid and Allows room walls
    // - Terrain type of neighbour tile is Room and it has Walls
    // So geometry for this wall should be managed by room
    bool NeighbourRoomHandlesWalls(TerrainTile* terrainTile, eDirection direction) const;
    bool RoomHandlesWalls(TerrainTile* terrainTile) const;

    // test whether neighbour tile has same room instance constructed on it
    bool NeighbourHasSameRoom(TerrainTile* terrainTile, eDirection direction) const;

private:
    // construction
    void ConstructTerrainWalls(TerrainTile* terrainTile);
    void ConstructTerrainWall(TerrainTile* terrainTile, eTileFace faceid);
    void ConstructTerrainFloor(TerrainTile* terrainTile);
    void ConstructTerrainQuad(TerrainTile* terrainTile, ArtResource* artResource);
    void ConstructTerrainWaterBed(TerrainTile* terrainTile, ArtResource* artResource);

    // test whether side wall should be constructed
    // @param dungeonMapTile: Target map tile
    // @param direction: Wall side
    bool ShouldBuildSideWall(TerrainTile* terrainTile, eTileFace faceid) const;
};