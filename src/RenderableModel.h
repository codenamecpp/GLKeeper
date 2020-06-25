#pragma once

#include "SceneDefs.h"
#include "SceneObject.h"

// renderable model scene object
class RenderableModel: public SceneObject
{
    decl_rtti(RenderableModel, SceneObject)

    friend class AnimModelsRenderer;

public:
    // readonly
    ModelAsset* mModelAsset = nullptr;

    std::vector<std::vector<Texture2D*>> mSubmeshTextures; // additional textures
    BlendFramesAnimState mAnimState;

    int mPreferredLOD = 0;

public:
    RenderableModel();

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

    // override RenderableObject methods
    void PrepareRenderResources() override;
    void ReleaseRenderResources() override;
    void RenderFrame(SceneRenderContext& renderContext) override;
    void UpdateFrame(float deltaTime) override;

private:
    void SetAnimationState();
    void SetLocalBounds();
};