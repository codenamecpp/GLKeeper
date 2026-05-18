#pragma once

#include "GraphicsDefs.h"

// Defines hardware 2d texture object
class GpuTexture2D: public cxx::noncopyable
{
    friend class RenderDevice;

private:
    
    //////////////////////////////////////////////////////////////////////////
    class ScopedBinder
    {
    public:
        ScopedBinder(GpuTexture2D* texture);
        ~ScopedBinder();
    private:
        GpuTexture2D* mPreviousTexture = nullptr;
        GpuTexture2D* mTexture = nullptr;
    };
    //////////////////////////////////////////////////////////////////////////

public:
    GpuTexture2D();
    ~GpuTexture2D();

    // Create texture of specified format and upload pixels data, no mipmaps
    // @param textureFormat: Format
    // @param sizex, sizey: Texture dimensions, must be POT!
    // @param sourceData: Source data buffer
    bool Create(ePixelFormat pixelFormat, int sizex, int sizey, const void* sourceData);

    // Create texture of specified format with mipmaps
    // @param textureFormat: Format
    // @param numMipmaps: Number of mipmaps including base texture level
    // @param mipmaps: Mips entries including base texture level, all sizes must be POT!
    bool Create(ePixelFormat pixelFormat, int numMipmaps, const Texture2DMip* mipmaps);

    // Free texture data but dont destroy hardware object
    void Invalidate();

    // Uploads pixels data for first mipmap, size of source bitmap should match current texture dimensions
    // @param sourceData: Source data buffer
    bool Upload(const void* sourceData);

    // Set texture filter and wrap parameters
    // @param filtering: Filtering mode
    // @param repeating: Addressing mode
    void SetSamplerState(eTextureFiltering filtering, eTextureRepeating repeating);

    // Test whether texture is currently bound at specified texture unit
    // @param unitIndex: Index of texture unit
    bool IsTextureBound(eTextureUnit textureUnit) const;

    const Point2D& GetTextureDimensions() const { return mDimensions; }

    inline ePixelFormat GetTexturePixelFormat() const { return mPixelFormat; }
    inline bool IsTexturePixelFormat(ePixelFormat pixelFormat) const
    {
        return mPixelFormat == pixelFormat;
    }
    inline int GetTextureWidth() const { return mDimensions.x; }
    inline int GetTextureHeight() const { return mDimensions.y; }

private:
    // shared render device context data
    static eTextureUnit sCurrentTextureUnit;
    static GpuTexture2D* sCurrentTextures2D[eTextureUnit_COUNT];

private:
    GpuResourceHandle mResourceHandle;
    eTextureFiltering mFiltering;
    eTextureRepeating mRepeating;
    Point2D mDimensions;
    bool mHasMipmaps;
    ePixelFormat mPixelFormat;
};