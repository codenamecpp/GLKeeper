#pragma once

// represents current low level graphics device state which is does not intended for direct usage
class GraphicsDeviceContext
{
public:
    GraphicsDeviceContext() 
        : mMainVaoHandle()
        , mCurrentBuffers()
        , mCurrentTextureUnit(eTextureUnit_0)
        , mCurrentTextures()
        , mCurrentProgram()
    {
    }
public:

    struct TextureUnitState
    {
        // note: mutual exclusion is used for different texture types
        GpuBufferTexture* mBufferTexture = nullptr;
        GpuTexture2D* mTexture2D = nullptr;
        GpuTextureArray2D* mTextureArray2D = nullptr;
    };

    GpuVertexArrayHandle mMainVaoHandle;
    GpuBuffer* mCurrentBuffers[eBufferContent_COUNT];
    GpuProgram* mCurrentProgram;
    eTextureUnit mCurrentTextureUnit;
    TextureUnitState mCurrentTextures[eTextureUnit_COUNT];
};