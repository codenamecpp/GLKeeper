#pragma once

#include "GameDefs.h"
#include "ScenarioDefs.h"

// helper class for terrain mesh construction
class TerrainMeshBuilder
{
public:
    // rebuild terrain mesh for specific tile
    // @param mapTile: Map tile
    // @param faceid: Tile face identifier
    void BuildTileMesh(GameMapTile* mapTile);
    void BuildTileMesh(GameMapTile* mapTile, eTileFace faceid);

    // append geometry to specific tile face mesh
    // @param mapTile: Map tile
    // @param faceid: Tile face identifier
    // @param asset: Geometry asset
    // @param rot: Optional rotation matrix
    // @param trans: Optional translation matrix
    void ExtendTileMesh(GameMapTile* mapTile, eTileFace faceid, ModelAsset* asset, const glm::mat3* rot = 0, const glm::vec3* trans = 0);

private:
    // construction
    void ConstructTerrainWalls(GameMapTile* mapTile);
    void ConstructTerrainWall(GameMapTile* mapTile, eTileFace faceid);
    void ConstructTerrainFloor(GameMapTile* mapTile);
    void ConstructTerrainQuad(GameMapTile* mapTile, ArtResource* artResource);
    void ConstructTerrainWaterBed(GameMapTile* mapTile, ArtResource* artResource);

    // test whether side wall should be constructed
    // @param dungeonMapTile: Target map tile
    // @param direction: Wall side
    bool ShouldBuildSideWall(GameMapTile* mapTile, eTileFace faceid) const;

    // helpers
    bool NeighbourTileSolid(GameMapTile* mapTile, eDirection direction) const;
    bool NeighbourHasSameTerrain(GameMapTile* mapTile, eDirection direction) const;
    bool NeighbourHasSameBaseTerrain(GameMapTile* mapTile, eDirection direction) const;
};