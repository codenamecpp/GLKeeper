#pragma once

#include "RenderableComponent.h"

// procedural static mesh component of game object
class StaticMeshComponent: public RenderableComponent
{
    decl_rtti(StaticMeshComponent, RenderableComponent)

    friend class StaticMeshComponentRenderer;

public:
    StaticMeshComponent(GameObject* gameObject);

    void InvalidateMesh();
    bool IsMeshInvalidated() const;

    // clear mesh vertices
    void ClearMesh();

    // process render frame
    void RenderFrame(SceneRenderContext& renderContext) override;

private:
    
};