#pragma once

// defines hardware 2d texture object
class GpuTexture2D: public cxx::noncopyable
{
    friend class GraphicsDevice;

public:
    // readonly
    TextureSamplerState mSamplerState;
    Texture2D_Desc mDesc;

public:

    // create hardware texture object with specified format and upload data
    // @param textureDesc: Texture2d information
    // @param sourceData: Source data buffer, optional
    bool InitTextureObject(const Texture2D_Desc& textureDesc, const void* sourceData);

    // free hardware texture memory 
    void FreeTextureObject();

    // uploads pixels data
    // @param mipmapLevel: Specifies the level-of-detail number; level 0 is the base image level
    // @param offset: Specifies a texel offset within the texture array
    // @param dimensions: Specifies the dimensions of the texture subimage
    // @param sourceData: Specifies a pointer to the source data
    bool TexSubImage(int mipmapLevel, const Rect2D& textureRect, const void* sourceData);
    bool TexSubImage(int mipmapLevel, const Size2D& dimensions, const void* sourceData);
    bool TexSubImage(int mipmapLevel, const void* sourceData);

    // set texture filter and wrap parameters
    // @param samplerState: Params
    bool SetSamplerState(const TextureSamplerState& samplerState);

    // test whether texture is currently bound at specified texture unit
    // @param unitIndex: Index of texture unit
    bool IsTextureBound(eTextureUnit textureUnit) const;
    bool IsTextureBound() const;

    // test whether texture object is initialized
    bool IsInitialized() const;

private:
    class ScopeBinder;
    
    GpuTexture2D(GraphicsDeviceContext& graphicsContext);
    ~GpuTexture2D();

    void SetSamplerStateImpl();
    void GenerateMipmapsImpl();
    void SetUnbound();

private:
    GpuTextureHandle mResourceHandle;
    GraphicsDeviceContext& mGraphicsContext;
};