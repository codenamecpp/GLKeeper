#pragma once

#include "GraphicsDefs.h"

// defines hardware 2d texture object
class GpuTextureArray2D: public cxx::noncopyable
{
    friend class GraphicsDevice;

public:
    // readonly
    eTextureFilterMode mFiltering;
    eTextureWrapMode mRepeating;
    Size2D mSize;
    int mLayersCount;
    eTextureFormat mFormat;

public:
    GpuTextureArray2D(GraphicsDeviceContext& graphicsContext);
    ~GpuTextureArray2D();

    // create texture of specified format and upload pixels data, no mipmaps
    // @param textureFormat: Format
    // @param sizex, sizey: Texture dimensions, must be POT!
    // @param layersCount: Number of textures in array
    // @param sourceData: Source data buffer, all layers must be specified if not null
    bool Setup(eTextureFormat textureFormat, int sizex, int sizey, int layersCount, const void* sourceData);

    // uploads pixels data for layers, size of source bitmap should match current texture dimensions
    // @param startLayerIndex: First layer index
    // @param layersCount: Number of layers to upload
    // @param sourceData: Source data buffer
    bool Upload(int startLayerIndex, int layersCount, const void* sourceData);

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
    class ScopedTextureArray2DBinder;

    void SetSamplerStateImpl(eTextureFilterMode filtering, eTextureWrapMode repeating);
    void SetUnbound();

private:
    GpuTextureHandle mResourceHandle;
    GraphicsDeviceContext& mGraphicsContext;
};
