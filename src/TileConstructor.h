#pragma once

#include "GameDefs.h"
#include "MapTile.h"

//////////////////////////////////////////////////////////////////////////

class MeshAsset;

//////////////////////////////////////////////////////////////////////////

class TileConstructor: public cxx::noncopyable
{
public:
    TileConstructor(const ScenarioDefinition& scenarioDef);

    // build tile mesh
    void ConstructTile(MapTile* mapTile);
    void ConstructTile(MapTile* mapTile, eTileFace targetFace);

    // Append geometry piece to tile wall
    inline void ExtendTileWall(MapTile* mapTile, eDirection direction, MeshAsset* mesh, const glm::mat3* rot = 0, const glm::vec3* trans = 0)
    {
        eTileFace face;
        switch (direction)
        {
            case eDirection_N: face = eTileFace_SideN; break;
            case eDirection_E: face = eTileFace_SideE; break;
            case eDirection_S: face = eTileFace_SideS; break;
            case eDirection_W: face = eTileFace_SideW; break;
            default: return;
        }
        ExtendTileMesh(mapTile, face, mesh, rot, trans);
    }

    // Append geometry piece to tile floor
    inline void ExtendTileFloor(MapTile* mapTile, MeshAsset* mesh, const glm::mat3* rot = 0, const glm::vec3* trans = 0)
    {
        ExtendTileMesh(mapTile, eTileFace_Floor, mesh, rot, trans);
    }

    // Append geometry piece to tile
    // @param mapTile: Target dungeon map tile
    // @param geometry: Target geometry
    // @param piece: Source geometry piece
    // @param rotation: Piece rotation, optional
    // @param translation: Piece translation, optional
    void ExtendTileMesh(MapTile* mapTile, eTileFace face, MeshAsset* mesh, const glm::mat3* rotation = 0, const glm::vec3* translation = 0);

    const ScenarioDefinition& GetScenarioDefinition() const { return mScenarioDef; }

private:
    // Construction
    void ConstructWalls(MapTile* mapTile);
    void ConstructWall(MapTile* mapTile, eTileFace tileFace);
    void ConstructFloor(MapTile* mapTile);
    void ConstructQuad(MapTile* mapTile, ArtResourceDefinition* resource);
    void ConstructWaterBed(MapTile* mapTile, ArtResourceDefinition* resource);

private:
    const ScenarioDefinition& mScenarioDef;
};
