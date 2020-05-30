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
    void InvalidateMesh();
    bool IsMeshInvalidated() const;

    // override RenderableComponent methods
    void PrepareRenderResources() override;
    void ReleaseRenderResources() override;
    void RenderFrame(SceneRenderContext& renderContext) override;

private:
    // dirty flag indicates that geometry is invalid and must be reuploaded
    bool mMeshInvalidated; 
};