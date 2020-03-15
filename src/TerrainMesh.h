#pragma once

#include "SceneObject.h"

// terrain mesh instance
class TerrainMesh: public SceneObject
{
public:
    // readonly
    Rect2D mMapTerrainArea;

public:
    TerrainMesh();
    ~TerrainMesh();

    // request entity to register itself in render lists
    // @param renderList: Render lists
    void RegisterForRendering(SceneRenderList& renderList) override;

private:
};