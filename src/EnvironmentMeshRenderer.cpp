#include "stdafx.h"
#include "EnvironmentMeshRenderer.h"
#include "GameRenderManager.h"
#include "TextureManager.h"
#include "GameWorld.h"
#include "ShadersManager.h"

//////////////////////////////////////////////////////////////////////////

#define WATER_ANIMATION_FPS 9.2f
#define NUM_WATER_ANIMATION_FRAMES 32

//////////////////////////////////////////////////////////////////////////

bool EnvironmentMeshRenderer::Initialize()
{
    mShaderProgram = gShadersManager.GetProgramOfType<ShaderProgram_WaterLava>("mesh_water_lava");
    cxx_assert(mShaderProgram);
    if (mShaderProgram == nullptr)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot load water_lava shader program");
        return false;
    }

    mShaderProgram->InitRenderData();

    mLavaTexture = gTextureManager.GetTexture("Lava0");
    cxx_assert(mLavaTexture);
    mLavaTexture->InitRenderData(); // todo: ensure to initialize in render thread
    mLavaTexture->GetGpuTexturePtr()->SetSamplerState(eTextureFiltering_Trilinear, eTextureRepeating_Repeat);

    LoadWaterTextureFrames();

    mWaterTexture = mWaterFrames.mTextures[0]; // initial first texture
    return true;
}

void EnvironmentMeshRenderer::Shutdown()
{
    mShaderProgram = nullptr;
    mLavaTexture = nullptr;
    mWaterTexture = nullptr;
    mWaterFrames.mTextures.clear();
}

void EnvironmentMeshRenderer::BeginFrame(Camera& camera)
{
    // prepare render program
    mShaderProgram->SetViewProjectionMatrix(camera.mViewProjectionMatrix);
}

void EnvironmentMeshRenderer::BeginBatch()
{
    mShaderProgram->BindProgram();
}

void EnvironmentMeshRenderer::RenderInstance(eRenderPass renderPass, EnvironmentMeshObject& object)
{
    // set params
    const EnvironmentMeshObject::Params& objectParams = object.GetParams();
    mShaderProgram->SetWaterLavaParams(objectParams.mWaveTime, 
        objectParams.mWaveWidth, 
        objectParams.mWaveHeight, 
        objectParams.mWaterlineHeight);

    SurfaceMaterial material;
    material.mDiffuseTexture = object.IsLava() ? mLavaTexture : mWaterTexture;

    bool isTranslucent = (objectParams.mTranslucency < 1.0f);
    if (isTranslucent)
    {
        material.mRenderStates.EnableAlphaBlend(RENDER_STATES_BLENDMODE_ALPHA);
        material.mOpacity = objectParams.mTranslucency;
    }
    else
    {
        material.mRenderStates.mIsAlphaBlendEnabled = false;
    }
    // validate render pass
    cxx_assert(isTranslucent ? (renderPass == eRenderPass_Translucent) : (renderPass == eRenderPass_Opaque));
    material.BindMaterial(*mShaderProgram);

    object.PrepareRenderdata();

    // bind vertex attributes
    gRenderDevice.BindVertexBuffer(object.mGpuVertexBuffer.get());
    gRenderDevice.BindIndexBuffer(object.mGpuIndexBuffer.get());
    gRenderDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, eIndicesType_i32, 0, object.mMeshTris.size() * 3);
}

void EnvironmentMeshRenderer::EndBatch()
{

}

void EnvironmentMeshRenderer::EndFrame()
{
    float gameTime = gTime.GetTime(eGameClock::Gametime);

    UpdateWaterAnimation(gameTime);
}

void EnvironmentMeshRenderer::LoadWaterTextureFrames()
{
    mWaterFrames.mTextures.clear();
    mWaterFrames.mCurrentFrame = 0;

    // aquire textures
    for (int iTexture = 0; iTexture < NUM_WATER_ANIMATION_FRAMES; ++iTexture)
    {
        const std::string iTextureIndexString = std::to_string(iTexture);

        Texture* texture = gTextureManager.GetTexture("Water" + iTextureIndexString);
        cxx_assert(texture);
        texture->InitRenderData(); // todo: ensure to initialize in render thread
        texture->GetGpuTexturePtr()->SetSamplerState(eTextureFiltering_Trilinear, eTextureRepeating_Repeat);
        mWaterFrames.mTextures.push_back(texture);
    }
}

void EnvironmentMeshRenderer::UpdateWaterAnimation(float currentTime)
{
    const unsigned int NumAnimationTextures = mWaterFrames.mTextures.size();
    if (NumAnimationTextures == 0)
        return;

    // get current frame index based on frametime
    const unsigned int currentFrame = (unsigned int)(currentTime * WATER_ANIMATION_FPS) % NumAnimationTextures;
    // Change current texture
    if (mWaterFrames.mCurrentFrame != currentFrame)
    {
        mWaterTexture = mWaterFrames.mTextures[currentFrame];
        mWaterFrames.mCurrentFrame = currentFrame;
    }
}
