#pragma once

#include "GraphicsDefs.h"

// defines hardware 2d texture object
class GpuTexture2D: public cxx::noncopyable
{
    friend class GraphicsDevice;

public:
    // readonly
    eTextureFilterMode mFiltering;
    eTextureWrapMode mRepeating;
    Size2D mSize;
    eTextureFormat mFormat;

    int mMipmapsCount;

public:
    GpuTexture2D(GraphicsDeviceContext& graphicsContext);
    ~GpuTexture2D();

    // create texture of specified format and upload pixels data
    // @param textureFormat: Format
    // @param dimensions: Texture dimensions, must be POT!
    // @param sourceData: Source data buffer
    bool SetTextureData(eTextureFormat textureFormat, const Size2D& dimensions, const void* sourceData);
    // @param textureData: Texture data
    bool SetTextureData(const Texture2D_Data& textureData);

    // set max mipmap levels count for texture
    // @param mipmapsCount: New mipmaps count
    void SetMipmapsCount(int mipmapsCount);

    // uploads pixels data
    // @param mipLevel: Specifies the level-of-detail number; level 0 is the base image level
    // @param xoffset, yoffset: Specifies a texel offset within the texture array
    // @param sizex, sizey: Specifies the size of the texture subimage
    // @param sourceData: Specifies a pointer to the source data
    bool TexSubImage(int mipLevel, int xoffset, int yoffset, int sizex, int sizey, const void* sourceData);
    bool TexSubImage(int mipLevel, int sizex, int sizey, const void* sourceData);

    // set texture filter and wrap parameters
    // @param filtering: Filtering mode
    // @param repeating: Addressing mode
    void SetSamplerState(eTextureFilterMode filtering, eTextureWrapMode repeating);

    // test whether texture is currently bound at specified texture unit
    // @param unitIndex: Index of texture unit
    bool IsTextureBound(eTextureUnit textureUnit) const;
    bool IsTextureBound() const;

    // test whether texture is created
    bool IsTextureInited() const;

private:
    class ScopeBinder;

    void SetSamplerStateImpl(eTextureFilterMode filtering, eTextureWrapMode repeating);
    void SetUnbound();

private:
    GpuTextureHandle mResourceHandle;
    GraphicsDeviceContext& mGraphicsContext;
};