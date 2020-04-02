#pragma once

// defines hardware buffer texture object
class GpuBufferTexture: public cxx::noncopyable
{
    friend class GraphicsDevice;

public:
    // readonly
    GpuBufferHandle mBufferHandle;
    int mBufferLength;
    eTextureFormat mFormat;

public:
    GpuBufferTexture(GraphicsDeviceContext& graphicsContext);
    ~GpuBufferTexture();

    // create texture of specified format and upload pixels data
    // @param textureFormat: Format
    // @param dataLength: Data length in bytes
    // @param sourceData: Source data buffer
    bool Setup(eTextureFormat textureFormat, int dataLength, const void* sourceData);

    // uploads data, size of source data should match specified buffer length
    // @param dataOffset: Buffer storage offset in bytes
    // @param dataLength: Data length in bytes
    // @param sourceData: Source data buffer
    bool Upload(int dataOffset, int dataLength, const void* sourceData);

    // test whether texture is currently bound at specified texture unit
    // @param unitIndex: Index of texture unit
    bool IsTextureBound(eTextureUnit textureUnit) const;
    bool IsTextureBound() const;

    // test whether texture is created
    bool IsTextureInited() const;

private:
    class ScopeBinder;

    void SetUnbound();

private:
    GpuTextureHandle mResourceHandle;
    GraphicsDeviceContext& mGraphicsContext;
};
