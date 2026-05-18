#pragma once

//////////////////////////////////////////////////////////////////////////

#include "EnvironmentMeshObject.h"
#include "ShaderProgram.h"

//////////////////////////////////////////////////////////////////////////
// Water and Lava dedicated renderer
//////////////////////////////////////////////////////////////////////////

class EnvironmentMeshRenderer: public cxx::noncopyable
{
public:
    bool Initialize();
    void Shutdown();

    void BeginFrame(Camera& camera);
    void EndFrame();

    void BeginBatch();
    void EndBatch();

    void RenderInstance(eRenderPass renderPass, EnvironmentMeshObject& surface);

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
};