#pragma once

#include "GameObjectComponent.h"

// procedural static mesh component of game object
class StaticMeshComponent: public GameObjectComponent
{
    GAMEOBJECT_COMPONENT(eGameObjectComponent_StaticMesh)

public:
    StaticMeshComponent(GameObject* gameObject);
    ~StaticMeshComponent();

    // process render frame
    void RenderFrame(SceneRenderContext& renderContext) override;
};