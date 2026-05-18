#pragma once

//////////////////////////////////////////////////////////////////////////

#include "SceneObject.h"

//////////////////////////////////////////////////////////////////////////

// water or lava visual object

class EnvironmentMeshObject: public SceneObject
{
    friend class EnvironmentMeshRenderer;

public:

    //////////////////////////////////////////////////////////////////////////
    struct Params
    {
    public:
        float mTranslucency = 1.0f;
        float mWaveWidth {};
        float mWaveHeight = 1.0f;
        float mWaveFreq {};
        float mWaveTime {};
        float mWaterlineHeight {};
    };
    //////////////////////////////////////////////////////////////////////////

public:
    EnvironmentMeshObject();

    void ConfigureLava();
    void ConfigureWater();
    void ConfigureParams(const Params& params);
    void ConfigureMapTiles(cxx::span<MapTile*> mapTiles);
    void AppendMapTiles(cxx::span<MapTile*> mapTiles);
    void RemoveMapTiles(cxx::span<MapTile*> mapTiles);
    void RemoveMapTiles();

    inline cxx::span<MapTile*> GetMapTiles() const { return mCoveredTiles; }

    // surface type
    inline bool IsWater() const { return mIsWater == true;  }
    inline bool IsLava () const { return mIsWater == false; }

    const Params& GetParams() const { return mParams; }

    // override SceneObject
    void UpdateFrame(float deltaTime) override;
    void RegisterForRendering(SceneRenderLists& renderLists, float distanceToCamera2) override;

    // pool
    void OnRecycle() override;

private:
    void PrepareRenderdata();

    void RefreshLocalBounds();
    void InvalidateMesh();
    void ReBuildMesh();

private:
    Params mParams;
    std::vector<MapTile*> mCoveredTiles;

    // flags
    bool mIsWater = true;
    bool mMeshDirty = false;

    // mesh data
    using TrisContainer = std::vector<glm::ivec3>;
    TrisContainer mMeshTris;

    using VertsContainer = std::vector<WaterLavaVertex>;
    VertsContainer mMeshVerts;

    // render data
    std::unique_ptr<GpuVertexBuffer> mGpuVertexBuffer;
    std::unique_ptr<GpuIndexBuffer> mGpuIndexBuffer;
};
