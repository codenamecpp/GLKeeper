#include "pch.h"
#include "TerrainHeightField.h"

void TerrainHeightField::InitHeightField(const Point& dimensions)
{
    mDimensions = dimensions;
    mHeightCells.resize(dimensions.x * dimensions.y);
    ClearHeights();
}

bool TerrainHeightField::IsInitialized() const
{
    return !mHeightCells.empty();
}

void TerrainHeightField::Cleanup()
{
    mHeightCells.clear();
}

void TerrainHeightField::UpdateHeights(const TilesList& terrainTiles)
{
    for (TerrainTile* currTile: terrainTiles)
    {
        UpdateHeights(currTile);
    }
}

void TerrainHeightField::UpdateHeights(TerrainTile* terrainTile)
{
    if (terrainTile == nullptr)
    {
        debug_assert(false);
        return;
    }

    if (IsInitialized())
    {
        glm::vec3 blockCoordinate;
        GetTerrainBlockCoordinate(terrainTile->mTileLocation, blockCoordinate);

        const float MaxHeight = TERRAIN_BLOCK_HEIGHT + TERRAIN_FLOOR_LEVEL;
        const float MinHeight = 0.0f;

        float stepLength = TERRAIN_BLOCK_SIZE / (SubdividePointsCount * 1.0f);
        for (int iy = 0; iy < SubdividePointsCount; ++iy)
        for (int ix = 0; ix < SubdividePointsCount; ++ix)
        {
            glm::vec3 origin {blockCoordinate.x + (iy * stepLength), MaxHeight + 1.0f, blockCoordinate.y + (ix * stepLength) };
            cxx::ray3d ray;
            float h0 = ComputeTerrainHeight(terrainTile->mFaces[eTileFace_Ceiling], ray);
            float h1 = ComputeTerrainHeight(terrainTile->mFaces[eTileFace_Floor], ray);
            float height = glm::clamp((h0 > h1) ? h0 : h1, MinHeight, MaxHeight); // choose max height
            int cellOffset = (terrainTile->mTileLocation.y * mDimensions.x) + (terrainTile->mTileLocation.x);
            mHeightCells[cellOffset].mPoints[ix][iy] = static_cast<unsigned char>((height / MaxHeight) * 255.0f);
        }
    }
}

void TerrainHeightField::ClearHeights()
{
    const int HeightCells = (int) mHeightCells.size();
    // fill with min height value
    memset(mHeightCells.data(), 0, HeightCells * sizeof(HeightFieldCell));
}

float TerrainHeightField::GetTerrainHeight(float coordx, float coordz) const
{
    if (IsInitialized())
    {
        //const float MaxHeight = TERRAIN_BLOCK_HEIGHT + TERRAIN_FLOOR_LEVEL;
        //const float MinHeight = 0.0f;
        //if (GameMapTile* currentTile = gWorldState.mDungeonMap.GetTileFromCoord3d(coordx, coordz))
        //{
        //    unsigned char height = mData[currentTile->mTileLocation.y * mSizeTilesX + currentTile->mTileLocation.x];
        //    return height * (maxHeight / 255.0f);
        //}
        return TERRAIN_FLOOR_LEVEL;
    }
    return 0.0f;
}

float TerrainHeightField::ComputeTerrainHeight(const TileFaceData& sourceData, const cxx::ray3d& processRay) const
{
    debug_assert(IsInitialized());
    glm::vec3 outPoint;

    float height = 0.0f;
    for (const TileMesh& currentPiece: sourceData.mMeshArray)
    {
        for (const glm::ivec3& currentTriangle: currentPiece.mTriangles)
        { 
            bool found = cxx::intersects(processRay, 
                currentPiece.mVertices[currentTriangle[0]].mPosition,
                currentPiece.mVertices[currentTriangle[1]].mPosition,
                currentPiece.mVertices[currentTriangle[2]].mPosition, outPoint);
            if (!found)
                continue;

            if (outPoint[1] > height)
            {
                height = outPoint[1];
            }
        }
    } // for
    return height;
}