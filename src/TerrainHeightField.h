#pragma once

#include "Texture2D_Image.h"
#include "TerrainTile.h"
#include "SimpleTriangleMesh.h"

// holds information about terrain height
class TerrainHeightField
{
public:
    // setup internal data
    // @param dimensions: Game world dimensions in tiles
    void InitHeightField(const Point& dimensions);
    bool IsInitialized() const;

    // free allocated memory
    void Cleanup();

    // each time terrain mesh gets modified, the heightfield should be manually updated
    void UpdateHeights(const TilesList& terrainTiles);
    void UpdateHeights(TerrainTile* terrainTile);
    void ClearHeights();

    // get terrain height at specific coordinate
    // @param coordinate: Point in world space, 'y' component is ignored
    // @returns height:
    //  maximum height is TERRAIN_BLOCK_HEIGHT + TERRAIN_FLOOR_LEVEL
    //  minimum height is 0
    float GetTerrainHeight(const glm::vec3& coordinate) const;
    float GetTerrainHeight(float coordx, float coordz) const
    {
        const glm::vec3 coordinate(coordx, 0.0f, coordz);
        return GetTerrainHeight(coordinate);
    }

    // for debug purposes
    void GenerateDebugMesh(Vertex3D_TriMesh& outputMesh) const;

private:
    // internal computations
    float ComputeTerrainHeight(const TileFaceData& sourceData, const cxx::ray3d& processRay) const;

private:
    static const int SubdivideCount = 2;
    static const int SubdividePointsCount = SubdivideCount * 2 - 1;
    struct HeightFieldCell
    {
        // x/y
        float mPoints[SubdividePointsCount][SubdividePointsCount];
    };
    std::vector<HeightFieldCell> mHeightCells;
    Point mDimensions; // num tiles w x h
};