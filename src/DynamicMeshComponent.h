#pragma once

#include "RenderableComponent.h"

// procedural static mesh component of game object
class DynamicMeshComponent: public RenderableComponent
{
    decl_rtti(DynamicMeshComponent, RenderableComponent)

public:
    DynamicMeshComponent(GameObject* gameObject);

    // process render frame
    void RenderFrame(SceneRenderContext& renderContext) override;
};