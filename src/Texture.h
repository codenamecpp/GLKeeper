#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GpuTexture2D.h"
#include "AssetDefs.h"

//////////////////////////////////////////////////////////////////////////

class Texture : public cxx::noncopyable
{
public:
    Texture(const std::string& textureName, eTextureBacking textureBacking);

    const std::string& GetTextureName() const { return mTextureName; }

    // loads texture data from file; does not create render data automatically
    // will initialize default texture if loading fails
    // will convert texture dimensions to power-of-two
    void Load();

    void CreateDefaultWhite();
    void CreateDefaultBlack();
    void CreateDefault();

    // uploads system bits to backend; the texture must be inited beforehand
    // will init fallback render data on failure
    void InitRenderData();

    // reset system bits and destroy render data 
    void Purge();

    void BindTexture(eTextureUnit textureUnit);

    // texture status
    inline bool IsLoaded() const { return mIsLoaded || mIsDefaultTexture; }
    inline bool IsInAtlas() const { return mTextureBacking == eTextureBacking_Atlas; }
    inline bool IsDefaultTexture() const { return mIsDefaultTexture; }
    inline bool IsRenderDataInited() const { return mIsRenderDataInited; }

    const TextureRegion& GetTextureRegion() const { return mTextureRegion; }
    inline GpuTexture2D* GetGpuTexturePtr() const
    {
        return mGpuTextureResourceReference;
    }

    inline GpuTexture2D* AcquireTextureRenderData()
    {
        InitRenderData();
        return mGpuTextureResourceReference;
    }

    // in case system bits are in memory
    inline const BitmapImage& GetBitmapImage() const
    {
        return mImageBitmap;
    }

    // use source id for geometry batching
    inline TextureSourceId GetTextureSourceId() const { return mTextureSourceId; }

    inline ePixelFormat GetPixelFormat() const { return mPixelFormat; }

    inline bool HasMipmaps() const { return mHasMipmaps; }
    inline bool HasAlpha() const { return mHasAlpha; }

private:
    void PostLoad();
    bool LoadFromFile();
    bool ConvertNPOT();
    
private:
    std::string mTextureName;

    BitmapImage mImageBitmap;
    ePixelFormat mPixelFormat = ePixelFormat_Null;
    TextureSourceId mTextureSourceId {};
    eTextureBacking mTextureBacking {};

    std::unique_ptr<GpuTexture2D> mOwnedGpuTextureResource;

    TextureRegion mTextureRegion {};
    TextureAtlas* mTextureAtlas {};

    // if the texture is part of a larger atlas, it doesn't own the gpu texture resource
    GpuTexture2D* mGpuTextureResourceReference {}; 

    bool mIsLoaded = false;
    bool mIsDefaultTexture = false;
    bool mIsRenderDataInited = false;
    bool mHasMipmaps = false;
    bool mHasAlpha = false;
};

//////////////////////////////////////////////////////////////////////////