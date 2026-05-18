#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GpuTexture2D.h"

//////////////////////////////////////////////////////////////////////////

class Texture : public cxx::noncopyable
{
public:
    Texture(const std::string& textureName);

    const std::string& GetTextureName() const { return mTextureName; }

    // loads texture data from file; does not create render data automatically
    // will initialize default texture if loading fails
    // @param convertToNPOT: ensure texture dimensions to be power-of-two
    void Load(bool convertNPOT = false);

    // init fallback texture
    void InitDefault();

    // uploads system bits to backend; the texture must be inited beforehand
    // will init fallback render data on failure
    void InitRenderData();

    // reset system bits and destroy render data 
    void Purge();

    void BindTexture(eTextureUnit textureUnit);

    // returns true if either texture is loaded from file or it is the default one
    inline bool IsInited() const { return mIsLoadedFromFile || mIsDefaultTexture; }
    inline bool IsLoadedFromFile() const { return mIsLoadedFromFile; }
    inline bool IsDefaultTexture() const { return mIsDefaultTexture; }
    inline bool IsRenderDataInited() const { return mIsRenderDataInited; }

    inline GpuTexture2D* GetGpuTexturePtr() const
    {
        return mGpuTextureResource.get();
    }

    const Point2D& GetTextureDimensions() const { return mTextureDimensions; }
    const Point2D& GetImageDimensions() const { return mImageDimensions; }
    inline Rect2D GetImageRect() const
    {
        return {0, 0, mImageDimensions.x, mImageDimensions.y};
    }
    inline ePixelFormat GetPixelFormat() const { return mPixelFormat; }

    inline bool HasMipmaps() const { return mHasMipmaps; }
    inline bool WasResized() const { return mWasResized; }
    inline bool HasAlpha() const { return mHasAlpha; }

    // in case system bits are in memory
    inline const BitmapImage& GetBitmapImage() const
    {
        return mImageBitmap;
    }

private:
    bool LoadFromFile(bool convertNPOT);
    bool ConvertNPOT();
    
private:
    std::string mTextureName;

    BitmapImage mImageBitmap;
    ePixelFormat mPixelFormat = ePixelFormat_Null;

    std::unique_ptr<GpuTexture2D> mGpuTextureResource;
    Point2D mTextureDimensions; // real dimensions of gpu texture
    Point2D mImageDimensions; // image dimensions, in case of npot

    bool mIsDefaultTexture = false;
    bool mIsLoadedFromFile = false;
    bool mIsRenderDataInited = false;

    bool mHasMipmaps = false;
    bool mWasResized = false;
    bool mHasAlpha = false;
};