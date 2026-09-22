#pragma once

//////////////////////////////////////////////////////////////////////////

#include "EnvironmentMeshObject.h"
#include "ShaderProgram.h"

//////////////////////////////////////////////////////////////////////////
// Water and Lava dedicated renderer
//////////////////////////////////////////////////////////////////////////

class EnvironmentMeshRenderer: public ISceneObjectRenderer
{
public:
    bool Initialize();
    void Shutdown();

    // override ISceneObjectRenderer
    void BeginFrame() override;
    void EndFrame() override;
    void BeginBatch(Camera& camera) override;
    void EndBatch() override;
    void RenderInstance(eRenderPass currentPass, SceneObject* object) override;

private:
    void UpdateWaterAnimation(float currentTime);
    void LoadWaterTextureFrames();

private:
    using FrameTextures = std::vector<Texture*>;
    struct WaterFrames 
    {
        FrameTextures mTextures;
        unsigned int mCurrentFrame = 0;
    };
    Texture* mLavaTexture = nullptr;
    Texture* mWaterTexture = nullptr;
    WaterFrames mWaterFrames;

    ShaderProgram_WaterLava* mShaderProgram = nullptr;

    size_t mFrameBatchCounter {};
};