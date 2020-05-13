#pragma once

#include "GameObjectComponent.h"
#include "SceneDefs.h"

// animating model component of game object
class AnimatingModelComponent: public GameObjectComponent
{
    GAMEOBJECT_COMPONENT(eGameObjectComponent_AnimatingModel)

public:
    // readonly
    ModelAsset* mModelAsset = nullptr;

    std::vector<MeshMaterial> mSubmeshMaterials;
    std::vector<std::vector<Texture2D*>> mSubmeshTextures; // additional textures
    BlendFramesAnimState mAnimState;

    int mPreferredLOD = 0;

    ModelsRenderData* mRenderData = nullptr;

public:
    AnimatingModelComponent(GameObject* gameObject);
    ~AnimatingModelComponent();

    // process component update frame
    // @param deltaTime: Time since last update
    void UpdateFrame(float deltaTime) override;

    // process render frame
    void RenderFrame(SceneRenderContext& renderContext) override;

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

private:
    void SetAnimationState();
    void SetLocalBounds();
};