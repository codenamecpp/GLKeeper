#pragma once

#include "SceneDefs.h"
#include "RenderableComponent.h"

// animating model component of entity
class AnimatingMeshComponent: public RenderableComponent
{
    decl_rtti(AnimatingMeshComponent, RenderableComponent)

    friend class AnimatingMeshRenderer;

public:
    // readonly
    ModelAsset* mModelAsset = nullptr;

    std::vector<std::vector<Texture2D*>> mSubmeshTextures; // additional textures
    BlendFramesAnimState mAnimState;

    int mPreferredLOD = 0;

public:
    AnimatingMeshComponent(Entity* entity);

    // change model asset, setup bounds and materials
    // @param modelAsset: Source model data
    void SetModelAsset(ModelAsset* modelAsset);
    void SetModelAssetNull();

    // model animation control
    bool StartAnimation(float animationSpeed, bool loop);
    void ClearAnimation();
    void RewindToStart();
    void RewingToEnd();
    void AdvanceAnimation(float deltaTime);
    void SetAnimationPaused(bool isPaused);

    bool IsAnimationLoop() const;
    bool IsAnimationActive() const;
    bool IsAnimationFinish() const;
    bool IsAnimationPaused() const;
    bool IsStatic() const;

    void SetPreferredLOD(int lod);

    // process component update frame
    // @param deltaTime: Time since last update
    void UpdateComponent(float deltaTime) override;

    // override RenderableComponent methods
    void PrepareRenderResources() override;
    void ReleaseRenderResources() override;
    void RenderFrame(SceneRenderContext& renderContext) override;

private:
    void SetAnimationState();
    void SetLocalBounds();
};