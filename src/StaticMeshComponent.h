#pragma once

#include "GameObjectComponent.h"

// procedural static mesh component of game object
class StaticMeshComponent: public GameObjectComponent
{
public:
    StaticMeshComponent(GameObject* gameObject);
    ~StaticMeshComponent();

    // process render frame
    void RenderFrame(SceneRenderContext& renderContext) override;
};

decl_gameobject_component(StaticMeshComponent, eGameObjectComponent_StaticMesh)