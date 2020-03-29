#include "pch.h"
#include "WaterLavaMesh.h"
#include "SceneRenderList.h"
#include "GraphicsDevice.h"
#include "GpuBuffer.h"
#include "MapTile.h"

// limits
const int MaxWaterLavaMeshBufferSize = 1024 * 1024 * 2;

WaterLavaMesh::WaterLavaMesh()
    : mVertexCount()
    , mTriangleCount()
    , mMeshDirty()
    , mWaveTime()
{
    mDebugColor = Color32_Blue;
}

WaterLavaMesh::~WaterLavaMesh()
{
    DestroyRenderData();
}

void WaterLavaMesh::SetWaterLavaTiles(const TilesArray& tilesArray)
{
    mWaterLavaTiles = tilesArray;

    // setup bounds
    cxx::aabbox bounds;
    for (MapTile* currentTile: tilesArray)
    {
        cxx::aabbox currentTileBounds;
        GetMapBlockBounds(currentTile->mTileLocation, currentTileBounds);

        bounds.extend(currentTileBounds);
    }
    SetLocalBoundingBox(bounds);

    mMeshDirty = true;
}

void WaterLavaMesh::SetSurfaceParams(float translucency, float waveWidth, float waveHeight, float waveFreq, float waterlineHeight)
{
    mTranslucency = translucency;
    mWaveWidth = waveWidth;
    mWaveHeight = waveHeight;
    mWaveFreq = waveFreq;
    mWaterlineHeight = waterlineHeight;

    if (translucency < 1.0f)
    {
        mMaterial.mRenderStates.mIsAlphaBlendEnabled = true;
        mMaterial.mRenderStates.mBlendingMode = eBlendingMode_Alpha;
    }
    else
    {
        mMaterial.mRenderStates.mIsAlphaBlendEnabled = false;
    }
}

void WaterLavaMesh::SetSurfaceTexture(Texture2D* diffuseTexture)
{
    mMaterial.mDiffuseTexture = diffuseTexture;
}

void WaterLavaMesh::UpdateMesh()
{
    if (!mMeshDirty)
        return;

    PrepareRenderData();

    mMeshDirty = false;
}

void WaterLavaMesh::UpdateFrame(float deltaTime)
{
    mWaveTime += mWaveFreq * deltaTime;
}

void WaterLavaMesh::RegisterForRendering(SceneRenderList& renderList)
{
    UpdateMesh();
    if (mTranslucency < 1.0f)
    {
        renderList.RegisterObject(eRenderPass_Translucent, this);
    }
    else
    {
        renderList.RegisterObject(eRenderPass_Opaque, this);
    }
}

void WaterLavaMesh::PrepareRenderData()
{
    if (mWaterLavaTiles.empty())
    {
        debug_assert(false);
        return;
    }

    const int NumVerticesPerTile = 9;
    const int NumTrianglesPerTile = 8;

    mVertexCount = mWaterLavaTiles.size() * NumVerticesPerTile;
    mTriangleCount = mWaterLavaTiles.size() * NumTrianglesPerTile;

    int actualVBufferLength = mVertexCount * Sizeof_Vertex3D_WaterLava;
    int actualIBufferLength = mTriangleCount * sizeof(glm::ivec3);
    if (actualVBufferLength > MaxWaterLavaMeshBufferSize || actualIBufferLength > MaxWaterLavaMeshBufferSize)
    {
        debug_assert(false);
        return;
    }

    // allocate vertex buffer object
    if (mVerticesBuffer == nullptr)
    {
        mVerticesBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Vertices);
        if (mVerticesBuffer == nullptr)
        {
            debug_assert(false);
            return;
        }
    }

    // allocate index buffer object
    if (mIndicesBuffer == nullptr)
    {
        mIndicesBuffer = gGraphicsDevice.CreateBuffer(eBufferContent_Indices);
        if (mIndicesBuffer == nullptr)
        {
            debug_assert(false);
            return;
        }
    }

    // setup buffers
    if (!mVerticesBuffer->Setup(eBufferUsage_Static, actualVBufferLength, nullptr) ||
        !mIndicesBuffer->Setup(eBufferUsage_Static, actualIBufferLength, nullptr))
    {
        debug_assert(false);
        return;
    }

    Vertex3D_WaterLava* vbufferPtr = mVerticesBuffer->LockData<Vertex3D_WaterLava>(BufferAccess_UnsynchronizedWrite, 0, actualVBufferLength);
    debug_assert(vbufferPtr);

    glm::ivec3* ibufferPtr = mIndicesBuffer->LockData<glm::ivec3>(BufferAccess_UnsynchronizedWrite, 0, actualIBufferLength);
    debug_assert(ibufferPtr);

    // upload data
    int vertices_counter = 0;
    for (MapTile* currTile: mWaterLavaTiles)
    {
        // setup indices
        const glm::ivec3 pointindices[NumTrianglesPerTile] = {
            {3, 4, 0}, {4, 1, 0}, // 1
            {4, 2, 1}, {4, 5, 2}, // 2
            {6, 4, 3}, {6, 7, 4}, // 3
            {7, 8, 4}, {8, 5, 4}, // 4
        };

        for (const glm::ivec3& pointindex : pointindices)
        {
            ibufferPtr->x = vertices_counter + pointindex.x;
            ibufferPtr->y = vertices_counter + pointindex.y;
            ibufferPtr->z = vertices_counter + pointindex.z;
            ++ibufferPtr;
        }

        // setup vertices
        const glm::vec3 middlep = 
        {
            currTile->mTileLocation.x * 1.0f, 0.0f, 
            currTile->mTileLocation.y * 1.0f
        };

        const glm::vec3 positions[NumVerticesPerTile] = 
        {
            {middlep.x - DUNGEON_CELL_HALF_SIZE, middlep.y, middlep.z - DUNGEON_CELL_HALF_SIZE},
            {middlep.x,                          middlep.y, middlep.z - DUNGEON_CELL_HALF_SIZE},
            {middlep.x + DUNGEON_CELL_HALF_SIZE, middlep.y, middlep.z - DUNGEON_CELL_HALF_SIZE},
            {middlep.x - DUNGEON_CELL_HALF_SIZE, middlep.y, middlep.z},
            middlep,
            {middlep.x + DUNGEON_CELL_HALF_SIZE, middlep.y, middlep.z},
            {middlep.x - DUNGEON_CELL_HALF_SIZE, middlep.y, middlep.z + DUNGEON_CELL_HALF_SIZE},
            {middlep.x,                          middlep.y, middlep.z + DUNGEON_CELL_HALF_SIZE},
            {middlep.x + DUNGEON_CELL_HALF_SIZE, middlep.y, middlep.z + DUNGEON_CELL_HALF_SIZE},
        };

        const glm::vec2 tcoordsp = {
            currTile->mTileLocation.x * 1.0f,
            currTile->mTileLocation.y * 1.0f
        };

        const glm::vec2 texturecoords[NumVerticesPerTile] = {
            tcoordsp,
            {tcoordsp.x + 0.5f, tcoordsp.y},
            {tcoordsp.x + 1.0f, tcoordsp.y},
            {tcoordsp.x, tcoordsp.y + 0.5f},
            {tcoordsp.x + 0.5f, tcoordsp.y + 0.5f},
            {tcoordsp.x + 1.0, tcoordsp.y + 0.5f},
            {tcoordsp.x, tcoordsp.y + 1.0f},
            {tcoordsp.x + 0.5f, tcoordsp.y + 1.0f},
            {tcoordsp.x + 1.0f, tcoordsp.y + 1.0f},
        };

        // process vertices
        for (int ipoint = 0; ipoint < NumVerticesPerTile; ++ipoint)
        {
            vbufferPtr->mPosition = positions[ipoint];
            vbufferPtr->mTexcoord = texturecoords[ipoint];
            ++vbufferPtr;
        }

        vertices_counter += NumVerticesPerTile;
    }

    if (!mIndicesBuffer->Unlock())
    {
        debug_assert(false);
    }

    if (!mVerticesBuffer->Unlock())
    {
        debug_assert(false);
    }
}

void WaterLavaMesh::DestroyRenderData()
{
    if (mVerticesBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mVerticesBuffer);
        mVerticesBuffer = nullptr;
        mVertexCount = 0;
    }

    if (mIndicesBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mIndicesBuffer);
        mIndicesBuffer = nullptr;
        mTriangleCount = 0;
    }

    mMeshDirty = false;
}
