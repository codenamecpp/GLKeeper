#pragma once

#include "RenderableComponent.h"

// terrain mesh component of game object
class TerrainMeshComponent: public RenderableComponent
{
    decl_rtti(TerrainMeshComponent, RenderableComponent)

    friend class TerrainMeshRenderer;

public:
    // readonly
    Rectangle mMapTerrainRect;

public:
    TerrainMeshComponent(GameObject* gameObject);

    void SetTerrainArea(const Rectangle& mapArea);

    // override RenderableComponent methods
    void PrepareRenderResources() override;
    void ReleaseRenderResources() override;
    void RenderFrame(SceneRenderContext& renderContext) override;
};