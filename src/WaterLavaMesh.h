#pragma once

#include "SceneObject.h"

// water or lava mesh instance
class WaterLavaMesh: public SceneObject
{
public:
    // readonly
public:
    WaterLavaMesh();
    ~WaterLavaMesh();

    // request entity to register itself in render lists
    // @param renderList: Render lists
    void RegisterForRendering(SceneRenderList& renderList) override;

private:
};