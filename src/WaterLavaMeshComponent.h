#pragma once

#include "RenderableComponent.h"

// water lava mesh component of entity
class WaterLavaMeshComponent: public RenderableComponent
{
    decl_rtti(WaterLavaMeshComponent, RenderableComponent)

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
    WaterLavaMeshComponent(Entity* entity);

    // set water or lava surface tiles
    // @param tilesArray: List of map tiles
    void SetWaterLavaTiles(const TilesList& tilesArray);

    // setup water or lava surface parameters
    void SetSurfaceParams(float translucency, float waveWidth, float waveHeight, float waveFreq, float waterlineHeight);

    // setup water or lava surface texture
    // @param diffuseTexture: Texture 2d
    void SetSurfaceTexture(Texture2D* diffuseTexture);

    // process update
    // @param deltaTime: Time since last update
    void UpdateComponent(float deltaTime) override;

    // override RenderableComponent methods
    void PrepareRenderResources() override;
    void ReleaseRenderResources() override;
    void RenderFrame(SceneRenderContext& renderContext) override;
};