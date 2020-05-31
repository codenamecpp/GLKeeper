#pragma once

#include "RenderableComponent.h"

// procedural static mesh component of game object
class UnlitMeshComponent: public RenderableComponent
{
    decl_rtti(UnlitMeshComponent, RenderableComponent)

public:
    UnlitMeshComponent(GameObject* gameObject);

    // process render frame
    void RenderFrame(SceneRenderContext& renderContext) override;
};