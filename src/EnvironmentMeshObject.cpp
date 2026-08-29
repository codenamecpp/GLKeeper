#include "stdafx.h"
#include "EnvironmentMeshObject.h"
#include "MapTile.h"
#include "MapUtils.h"

EnvironmentMeshObject::EnvironmentMeshObject()
{
}

void EnvironmentMeshObject::ConfigureLava()
{
    mIsWater = false;

    const float DFLT_SURF_LEVEL          = 0.82f;
    const float DFLT_SURF_TRANSLUCENCY   = 1.0f;
    const float DFLT_SURF_WAVE_WIDTH     = 0.76f;
    const float DFLT_SURF_WAVE_HEIGHT    = 0.15f;
    const float DFLT_SURF_WAVE_FREQ      = 2.5f;

    Params params;
    {
        params.mTranslucency     = DFLT_SURF_TRANSLUCENCY;
        params.mWaveWidth        = DFLT_SURF_WAVE_WIDTH;
        params.mWaveHeight       = DFLT_SURF_WAVE_HEIGHT;
        params.mWaveFreq         = DFLT_SURF_WAVE_FREQ;
        params.mWaterlineHeight  = DFLT_SURF_LEVEL;
    }
    // debug info
    mDebugColor = mIsWater ? COLOR_CYAN : COLOR_ORANGE;
    ConfigureParams(params); 
}

void EnvironmentMeshObject::ConfigureWater()
{
    mIsWater = true;

    const float DFLT_SURF_LEVEL         = 0.92f;
    const float DFLT_SURF_TRANSLUCENCY  = 0.65f;
    const float DFLT_SURF_WAVE_WIDTH    = 0.0f;
    const float DFLT_SURF_WAVE_HEIGHT   = 0.0f;
    const float DFLT_SURF_WAVE_FREQ     = 2.4f;

    Params params;
    {
        params.mTranslucency     = DFLT_SURF_TRANSLUCENCY;
        params.mWaveWidth        = DFLT_SURF_WAVE_WIDTH;
        params.mWaveHeight       = DFLT_SURF_WAVE_HEIGHT;
        params.mWaveFreq         = DFLT_SURF_WAVE_FREQ;
        params.mWaterlineHeight  = DFLT_SURF_LEVEL;
    }
    // debug info
    mDebugColor = mIsWater ? COLOR_CYAN : COLOR_ORANGE;
    ConfigureParams(params); 
}

void EnvironmentMeshObject::ConfigureParams(const Params& params)
{
    mParams = params;
}

void EnvironmentMeshObject::ConfigureMapTiles(cxx::span<MapTile*> mapTiles)
{
    if (mapTiles.empty())
    {
        RemoveMapTiles();
        return;
    }

    // remove old
    const size_t prevSize = mCoveredTiles.size();
    cxx::erase_if(mCoveredTiles, [&mapTiles](const MapTile* srcTile)
        {
            return !cxx::contains(mapTiles, srcTile);
        });

    if (prevSize != mCoveredTiles.size())
    {
        InvalidateMesh();
    }

    // add new
    for (MapTile* roller: mapTiles)
    {
        if (!cxx::contains(mCoveredTiles, roller))
        {
            mCoveredTiles.push_back(roller);
            InvalidateMesh();
        }
    }

    RefreshLocalBounds();
}

void EnvironmentMeshObject::AppendMapTiles(cxx::span<MapTile*> mapTiles)
{
    for (MapTile* roller: mapTiles)
    {
        if (!cxx::contains(mCoveredTiles, roller))
        {
            mCoveredTiles.push_back(roller);
            InvalidateMesh();
        }
    }

    RefreshLocalBounds();
}

void EnvironmentMeshObject::RemoveMapTiles(cxx::span<MapTile*> mapTiles)
{
    // remove old
    const size_t prevSize = mCoveredTiles.size();
    cxx::erase_if(mCoveredTiles, [&mapTiles](const MapTile* srcTile)
    {
        return !cxx::contains(mapTiles, srcTile);
    });

    if (prevSize != mCoveredTiles.size())
    {
        InvalidateMesh();
    }

    RefreshLocalBounds();
}

void EnvironmentMeshObject::RemoveMapTiles()
{
    if (mCoveredTiles.empty())
        return;

    mCoveredTiles.clear();
    InvalidateMesh();
    RefreshLocalBounds();
}

void EnvironmentMeshObject::OnRecycle()
{
    SceneObject::OnRecycle();

    mMeshDirty = false;
    mCoveredTiles.clear();
    mMeshTris.clear();
    mMeshVerts.clear();
    mGpuVertexBuffer.reset();
    mGpuIndexBuffer.reset();
}

void EnvironmentMeshObject::UpdateFrame(float deltaTime)
{
    // animate waves
    mParams.mWaveTime += mParams.mWaveFreq * deltaTime;
}

void EnvironmentMeshObject::RegisterForRendering(SceneRenderLists& renderLists, float distanceToCamera2)
{
    if (mCoveredTiles.empty()) return;

    eRenderPass targetPass = (mParams.mTranslucency < 1.0f) ? eRenderPass_Translucent : eRenderPass_Opaque;
    renderLists.Register(targetPass, this, distanceToCamera2);
}

void EnvironmentMeshObject::PrepareRenderdata()
{
    if (!mMeshDirty)
        return;

    mMeshDirty = false;
    ReBuildMesh();

    if (mMeshTris.empty() || mMeshVerts.empty())
        return;

    // uploading tris to gpu
    const unsigned int Sizeof_Triangle = sizeof(glm::ivec3);
    const unsigned int TrisBufferLength = Sizeof_Triangle * mMeshTris.size();
    // allocate new
    if (!mGpuIndexBuffer)
    {
        mGpuIndexBuffer = gRenderDevice.CreateIndexBuffer(eBufferUsage_Static, TrisBufferLength);
        cxx_assert(mGpuIndexBuffer);
        if (mGpuIndexBuffer && !mGpuIndexBuffer->SubData(0, TrisBufferLength, mMeshTris.data()))
        {
            cxx_assert(false);
        }
    }
    // refresh
    else if (!mGpuIndexBuffer->Create(eBufferUsage_Static, TrisBufferLength, mMeshTris.data()))
    {
        cxx_assert(false);
    }

    // uploading verts to gpu
    const unsigned int VertsBufferLength = Sizeof_WaterLavaVertex * mMeshVerts.size();
    // allocate new
    if (!mGpuVertexBuffer)
    {
        mGpuVertexBuffer = gRenderDevice.CreateVertexBuffer(eBufferUsage_Static, VertsBufferLength);
        if (mGpuVertexBuffer && mGpuVertexBuffer->SubData(0, VertsBufferLength, mMeshVerts.data()))
        {
            mGpuVertexBuffer->ConfigureVertexDefinitions(WaterLavaVertex3D_Format::Get());
        }
        else
        {
            cxx_assert(false);
        }
    }
    // refresh
    else if (!mGpuVertexBuffer->Create(eBufferUsage_Static, VertsBufferLength, mMeshVerts.data()))
    {
        cxx_assert(false);
    }
}

void EnvironmentMeshObject::RefreshLocalBounds()
{
    bool wasActive = IsObjectActive();
    // force apply bounds
    SetObjectActive(false);

    cxx::aabbox bounds;
    bounds.set_to_zero();

    if (!mCoveredTiles.empty())
    {
        Point2D minTilePos = mCoveredTiles.front()->mLocation;
        Point2D maxTilePos = mCoveredTiles.front()->mLocation;

        for (MapTile* currentTile: mCoveredTiles)
        {
            minTilePos.x = std::min(minTilePos.x, currentTile->mLocation.x);
            minTilePos.y = std::min(minTilePos.y, currentTile->mLocation.y);

            maxTilePos.x = std::max(maxTilePos.x, currentTile->mLocation.x);
            maxTilePos.y = std::max(maxTilePos.y, currentTile->mLocation.y);
        }

        bounds = MapUtils::ComputeBlockBounds(minTilePos);
        bounds.extend(MapUtils::ComputeBlockBounds(maxTilePos));
    }

    SetLocalBoundingBox(bounds);
    SetObjectActive(wasActive);
}

void EnvironmentMeshObject::InvalidateMesh()
{
    mMeshDirty = true;
}

void EnvironmentMeshObject::ReBuildMesh()
{
    // process data
    mMeshTris.clear();
    mMeshVerts.clear();

    if (mCoveredTiles.empty())
        return;

    // generate geometry
    for (MapTile* tile : mCoveredTiles)
    {
        const Point2D& mapPos = tile->mLocation;
        const glm::vec3 middlep = { mapPos.x * 1.0f, 0.0f, mapPos.y * 1.0f };

        const glm::vec3 positions[9] = {
            {middlep.x - MAP_TILE_HALF_SIZE,    middlep.y, middlep.z - MAP_TILE_HALF_SIZE},
            {middlep.x,                         middlep.y, middlep.z - MAP_TILE_HALF_SIZE},
            {middlep.x + MAP_TILE_HALF_SIZE,    middlep.y, middlep.z - MAP_TILE_HALF_SIZE},
            {middlep.x - MAP_TILE_HALF_SIZE,    middlep.y, middlep.z},
            middlep,
            {middlep.x + MAP_TILE_HALF_SIZE,    middlep.y, middlep.z},
            {middlep.x - MAP_TILE_HALF_SIZE,    middlep.y, middlep.z + MAP_TILE_HALF_SIZE},
            {middlep.x,                         middlep.y, middlep.z + MAP_TILE_HALF_SIZE},
            {middlep.x + MAP_TILE_HALF_SIZE,    middlep.y, middlep.z + MAP_TILE_HALF_SIZE},
        };

        const glm::vec2 tcoordsp = { mapPos.x * 1.0f, mapPos.y * 1.0f };

        const glm::vec2 texturecoords[9] = {
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

        int32_t tile_vert_indices[9];

        // process vertices
        for (int ipoint = 0; ipoint < 9; ++ipoint)
        {
            const glm::vec3& pointPos = positions[ipoint];

            // find same vertex
            // todo: optimize
            int32_t vidx = cxx::get_first_index_if(mMeshVerts, [&pointPos](const WaterLavaVertex& v)
                {
                    return v.mPosition == pointPos;
                });

            if (vidx == -1)
            {
                // allocate new
                tile_vert_indices[ipoint] = static_cast<int32_t>(mMeshVerts.size());
                WaterLavaVertex& meshVertex = mMeshVerts.emplace_back();
                meshVertex.mPosition = positions[ipoint];
                meshVertex.mTexcoord = texturecoords[ipoint];
            }
            else
            {
                // reuse vertex
                tile_vert_indices[ipoint] = vidx;
            }
        }

        // create triangles
        const glm::ivec3 pointindices[8] = {
            {3, 4, 0}, {4, 1, 0}, // 1
            {4, 2, 1}, {4, 5, 2}, // 2
            {6, 4, 3}, {6, 7, 4}, // 3
            {7, 8, 4}, {8, 5, 4}, // 4
        };

        for (const glm::ivec3& pointindex : pointindices)
        {
            mMeshTris.emplace_back(
                tile_vert_indices[pointindex.x], 
                tile_vert_indices[pointindex.y], 
                tile_vert_indices[pointindex.z]);
        }
    }

    int bp = 0;
}
