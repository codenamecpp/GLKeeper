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

        const float stepLength = TERRAIN_BLOCK_SIZE / (SubdivideCount * 1.0f);
        for (int iy = 0; iy < SubdividePointsCount; ++iy)
        for (int ix = 0; ix < SubdividePointsCount; ++ix)
        {
            cxx::ray3d ray;
            ray.mDirection = -SceneAxisY;
            ray.mOrigin[0] = blockCoordinate.x + (ix * stepLength);
            ray.mOrigin[1] = MaxHeight + 1.0f;
            ray.mOrigin[2] = blockCoordinate.z + (iy * stepLength);
            float h0 = ComputeTerrainHeight(terrainTile->mFaces[eTileFace_Ceiling], ray);
            float h1 = ComputeTerrainHeight(terrainTile->mFaces[eTileFace_Floor], ray);
            float height = glm::clamp((h0 > h1) ? h0 : h1, MinHeight, MaxHeight); // choose max height
            int cellOffset = (terrainTile->mTileLocation.y * mDimensions.x) + (terrainTile->mTileLocation.x);
            mHeightCells[cellOffset].mPoints[ix][iy] = height;
        }
    }
}

void TerrainHeightField::ClearHeights()
{
    const int HeightCells = (int) mHeightCells.size();
    // fill with min height value
    memset(mHeightCells.data(), 0, HeightCells * sizeof(HeightFieldCell));
}

float TerrainHeightField::GetTerrainHeight(const glm::vec3& coordinate) const
{
    const float StepLength = TERRAIN_BLOCK_SIZE / (SubdivideCount * 1.0f);
    const float MaxHeight = TERRAIN_BLOCK_HEIGHT + TERRAIN_FLOOR_LEVEL;

    if (IsInitialized())
    {
        glm::vec3 coordinateWithinTile;
        GetCoordinateWithinTerrainBlock(coordinate, coordinateWithinTile);

        Point tileLocation;
        GetTerrainBlockLocation(coordinate, tileLocation);

        tileLocation.x = glm::clamp(tileLocation.x, 0, mDimensions.x - 1);
        tileLocation.y = glm::clamp(tileLocation.y, 0, mDimensions.y - 1);

        const HeightFieldCell& cell = mHeightCells[tileLocation.y * mDimensions.x + tileLocation.x];

        // todo: optimize this!

        //  0,0        1,0
        //    A ------ B
        //    |     // |
        //    |   //   |
        //    | //     |
        //    C ------ D
        //  0,1        1,1

        cxx::ray3d ray;
        ray.mOrigin.x = coordinateWithinTile.x;
        ray.mOrigin.y = MaxHeight + 1.0f;
        ray.mOrigin.z = coordinateWithinTile.z;
        ray.mDirection = -SceneAxisY;

        glm::vec3 output;
        for (int cell_y = 0; cell_y < SubdivideCount; ++cell_y)
        for (int cell_x = 0; cell_x < SubdivideCount; ++cell_x)
        {
            glm::vec3 pA ((cell_x + 0) * StepLength, cell.mPoints[cell_x + 0][cell_y + 0], (cell_y + 0) * StepLength);
            glm::vec3 pB ((cell_x + 1) * StepLength, cell.mPoints[cell_x + 1][cell_y + 0], (cell_y + 0) * StepLength);
            glm::vec3 pC ((cell_x + 0) * StepLength, cell.mPoints[cell_x + 0][cell_y + 1], (cell_y + 1) * StepLength);
            glm::vec3 pD ((cell_x + 1) * StepLength, cell.mPoints[cell_x + 1][cell_y + 1], (cell_y + 1) * StepLength);

            // upper triangle
            if (cxx::intersects(ray, pA, pC, pB, output))
            {
                return output.y;
            }
            // lower triangle
            if (cxx::intersects(ray, pC, pD, pB, output))
            {
                return output.y;
            }
        }
        return TERRAIN_FLOOR_LEVEL;
    }
    return 0.0f;
}

void TerrainHeightField::GenerateDebugMesh(Vertex3D_TriMesh& outputMesh) const
{
    const float StepLength = TERRAIN_BLOCK_SIZE / (SubdivideCount * 1.0f);
    const float MaxHeight = TERRAIN_BLOCK_HEIGHT + TERRAIN_FLOOR_LEVEL;

    outputMesh.Clear();
    outputMesh.mVertices.reserve(mDimensions.x * mDimensions.y * SubdividePointsCount * SubdividePointsCount);
    outputMesh.mTriangles.reserve(mDimensions.x * mDimensions.y * SubdivideCount * SubdivideCount * 2);
    for (int iy = 0; iy < mDimensions.y; ++iy)
    for (int ix = 0; ix < mDimensions.x; ++ix)
    {
        const HeightFieldCell& cell = mHeightCells[iy * mDimensions.x + ix];

        glm::vec3 blockCoordinate;
        GetTerrainBlockCoordinate(Point (ix, iy), blockCoordinate);

        int baseVertex = (int) outputMesh.mVertices.size();

        // add vertices
        for (int pointy = 0; pointy < SubdividePointsCount; ++pointy)
        for (int pointx = 0; pointx < SubdividePointsCount; ++pointx)
        {
            Vertex3D vertex;
            vertex.mPosition.x = blockCoordinate.x + (pointx * StepLength);
            vertex.mPosition.y = 0.024f + cell.mPoints[pointx][pointy];
            vertex.mPosition.z = blockCoordinate.z + (pointy * StepLength);
            outputMesh.mVertices.push_back(vertex);
        }

        for (int quady = 0; quady < SubdivideCount; ++quady)
        for (int quadx = 0; quadx < SubdivideCount; ++quadx)
        {
            glm::ivec3 triangle0 {
                baseVertex + quadx + (quady * SubdividePointsCount), 
                baseVertex + quadx + ((quady + 1) * SubdividePointsCount), 
                baseVertex + (quadx + 1) + (quady * SubdividePointsCount),
            };
            outputMesh.mTriangles.push_back(triangle0);
            glm::ivec3 triangle1 {
                triangle0[2], 
                triangle0[1], 
                baseVertex + (quadx + 1) + ((quady + 1) * SubdividePointsCount),
            };
            outputMesh.mTriangles.push_back(triangle1);
        }
    }
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