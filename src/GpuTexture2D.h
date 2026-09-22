#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GraphicsDefs.h"

//////////////////////////////////////////////////////////////////////////

class GpuTexture2D: public cxx::noncopyable
{
    friend class RenderDevice;

public:
    
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

    //////////////////////////////////////////////////////////////////////////
    // texture data initialization
    //////////////////////////////////////////////////////////////////////////

    // texture size must be POT!

    bool Create(const BitmapImage& picture, 
        eTextureFiltering filter = eTextureFiltering_Bilinear, 
        eTextureRepeating repeat = eTextureRepeating_ClampToEdge);

    bool Create(int numMipmaps, const Texture2DMip* mipmaps, ePixelFormat pixelFormat, 
        eTextureFiltering filter = eTextureFiltering_Bilinear, 
        eTextureRepeating repeat = eTextureRepeating_ClampToEdge);

    // no mipmaps
    bool Create(const Point2D& dimensions, ePixelFormat pixelFormat, const void* pixeldata,
        eTextureFiltering filter = eTextureFiltering_Bilinear, 
        eTextureRepeating repeat = eTextureRepeating_ClampToEdge);

    //////////////////////////////////////////////////////////////////////////
    // texture data update
    //////////////////////////////////////////////////////////////////////////
    
    void Upload(const Point2D& offset, const Point2D& dimensions, const void* sourceData, int mipLevel = 0);
    void Upload(const Rect2D& textureArea, const void* sourceData, int mipLevel = 0);
    void Upload(const void* sourceData, int mipLevel = 0);

    //////////////////////////////////////////////////////////////////////////

    // Set texture filter and wrap parameters
    void SetSamplerState(eTextureFiltering filtering, eTextureRepeating repeating);

    // Test whether texture is currently bound at specified texture unit
    bool IsBound(eTextureUnit textureUnit) const;

    //////////////////////////////////////////////////////////////////////////
    // info
    //////////////////////////////////////////////////////////////////////////

    const Point2D& GetDimensions() const { return mDimensions; }
    inline int GetWidth() const { return mDimensions.x; }
    inline int GetHeight() const { return mDimensions.y; }

    inline ePixelFormat GetPixelFormat() const { return mPixelFormat; }
    inline bool HasPixelFormat(ePixelFormat pixelFormat) const
    {
        return mPixelFormat == pixelFormat;
    }
private:
    void SetParams(eTextureFiltering filtering, eTextureRepeating repeating);
private:
    //////////////////////////////////////////////////////////////////////////
    // shared render device context data
    static eTextureUnit sCurrentTextureUnit;
    static GpuTexture2D* sCurrentTextures2D[eTextureUnit_COUNT];
    //////////////////////////////////////////////////////////////////////////
private:
    GpuResourceHandle mResourceHandle;
    eTextureFiltering mFiltering;
    eTextureRepeating mRepeating;
    ePixelFormat mPixelFormat;
    Point2D mDimensions;
    bool mHasMipmaps;
};

//////////////////////////////////////////////////////////////////////////