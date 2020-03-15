#pragma once

#include "SceneObject.h"

// water or lava mesh instance
class WaterLavaMesh: public SceneObject
{
public:
    // readonly
    float mTranslucency = 1.0f;
    float mWaveWidth;
    float mWaveHeight;
    float mWaveFreq;
    float mWaveTime;
    float mWaterlineHeight = 1.0;

public:
    WaterLavaMesh();
    ~WaterLavaMesh();

    // process scene update frame
    // @param deltaTime: Time since last update
    void UpdateFrame(float deltaTime) override;

    // request entity to register itself in render lists
    // @param renderList: Render lists
    void RegisterForRendering(SceneRenderList& renderList) override;

private:
};