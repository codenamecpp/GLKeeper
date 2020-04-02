#pragma once

#include "GameObjectComponent.h"
#include "RenderMaterial.h"

// water lava mesh component of game object
class WaterLavaMeshComponent: public GameObjectComponent
{
    friend class WaterLavaMeshRenderer;
    friend class RenderManager;

public:
    // readonly
    float mTranslucency = 1.0f;
    float mWaveWidth;
    float mWaveHeight;
    float mWaveFreq;
    float mWaveTime;
    float mWaterlineHeight = 1.0;

    TilesArray mWaterLavaTiles;

public:
    WaterLavaMeshComponent(GameObject* gameObject);
    ~WaterLavaMeshComponent();

    // process game object update frame
    // @param deltaTime: Time since last update
    void UpdateFrame(float deltaTime) override;

    // process render frame
    void RenderFrame(SceneRenderContext& renderContext) override;

    // set water or lava surface tiles
    // @param tilesArray: List of map tiles
    void SetWaterLavaTiles(const TilesArray& tilesArray);

    // setup water or lava surface parameters
    void SetSurfaceParams(float translucency, float waveWidth, float waveHeight, float waveFreq, float waterlineHeight);

    // setup water or lava surface texture
    // @param diffuseTexture: Texture 2d
    void SetSurfaceTexture(Texture2D* diffuseTexture);

    // rebuild water lava mesh and upload data to video memory
    void UpdateMesh();

    void ClearMesh();

private:
    void PrepareRenderData();
    void DestroyRenderData();

private:
    // render data
    RenderMaterial mMaterial;

    GpuBuffer* mVerticesBuffer = nullptr;
    GpuBuffer* mIndicesBuffer = nullptr;

    int mVertexCount;
    int mTriangleCount;

    bool mMeshDirty; // dirty flag indicates that geometry is invalid and must be reuploaded
};