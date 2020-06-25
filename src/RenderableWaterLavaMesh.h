#pragma once

#include "SceneObject.h"

// water lava mesh scene object
class RenderableWaterLavaMesh: public SceneObject
{
    decl_rtti(RenderableWaterLavaMesh, SceneObject)

    friend class WaterLavaMeshRenderer;

public:
    // readonly
    float mTranslucency = 1.0f;
    float mWaveWidth;
    float mWaveHeight;
    float mWaveFreq;
    float mWaveTime;
    float mWaterlineHeight = 1.0;

    TilesList mWaterLavaTiles;

public:
    RenderableWaterLavaMesh();

    // set water or lava surface tiles
    // @param tilesArray: List of map tiles
    void SetWaterLavaTiles(const TilesList& tilesArray);

    // setup water or lava surface parameters
    void SetSurfaceParams(float translucency, float waveWidth, float waveHeight, float waveFreq, float waterlineHeight);

    // setup water or lava surface texture
    // @param diffuseTexture: Texture 2d
    void SetSurfaceTexture(Texture2D* diffuseTexture);

    // override RenderableObject methods
    void PrepareRenderResources() override;
    void ReleaseRenderResources() override;
    void RenderFrame(SceneRenderContext& renderContext) override;
    void UpdateFrame(float deltaTime) override;
};