#pragma once

#include "SceneObject.h"

// terrain mesh scene object
class RenderableTerrainMesh: public SceneObject
{
    decl_rtti(RenderableTerrainMesh, SceneObject)

    friend class TerrainMeshRenderer;

public:
    // readonly
    Rectangle mMapTerrainRect;

public:
    RenderableTerrainMesh();

    void SetTerrainArea(const Rectangle& mapArea);

    // override RenderableObject methods
    void PrepareRenderResources() override;
    void ReleaseRenderResources() override;
    void RenderFrame(SceneRenderContext& renderContext) override;
};