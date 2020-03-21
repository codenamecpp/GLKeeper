#pragma once

#include "GameDefs.h"
#include "ScenarioDefs.h"

// helper class for terrain mesh construction
class TerrainMeshBuilder
{
public:
    // rebuild terrain mesh for specific tile
    // @param tile: Map tile
    // @param faceid: Tile face identifier
    void BuildTileMesh(MapTile* tile);
    void BuildTileMesh(MapTile* tile, eTileFace faceid);

    // append geometry to specific tile face mesh
    // @param tile: Map tile
    // @param faceid: Tile face identifier
    // @param asset: Geometry asset
    // @param rot: Optional rotation matrix
    // @param trans: Optional translation matrix
    void ExtendTileMesh(MapTile* tile, eTileFace faceid, ModelAsset* asset, const glm::mat3* rot = 0, const glm::vec3* trans = 0);

private:
    // construction
    void ConstructTerrainWalls(MapTile* tile);
    void ConstructTerrainWall(MapTile* tile, eTileFace faceid);
    void ConstructTerrainFloor(MapTile* tile);
    void ConstructTerrainQuad(MapTile* tile, ArtResource* artResource);
    void ConstructTerrainWaterBed(MapTile* tile, ArtResource* artResource);

    // test whether side wall should be constructed
    // @param dungeonMapTile: Target map tile
    // @param direction: Wall side
    bool ShouldBuildSideWall(MapTile* tile, eTileFace faceid) const;

    // helpers
    bool NeighbourTileSolid(MapTile* tile, eDirection direction) const;
    bool NeighbourHasSameTerrain(MapTile* tile, eDirection direction) const;
    bool NeighbourHasSameBaseTerrain(MapTile* tile, eDirection direction) const;
};