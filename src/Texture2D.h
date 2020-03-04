#pragma once

#include "GraphicsDefs.h"

// texture 2d resource
class Texture2D: public cxx::noncopyable
{
public:
    // readonly
    std::string mTextureName;

    TextureSamplerState mSamplerState;

    GpuTexture2D* mGpuTextureObject = nullptr;

public:
    Texture2D(const std::string& textureName);
    ~Texture2D();

    // load or free texture data
    bool LoadTexture();
    void FreeTexture();

    // create texture from source data
    // @param textureData: Data
    // @returns false on error
    bool CreateTexture(const Texture2D_Data& textureData);

    // set texture filtering and repeating modes
    // @param samplerState: Sampler state params
    void SetSamplerState(const TextureSamplerState& samplerState);

    // test whether texture is initialized and currently active
    bool IsTextureLoaded() const;
    bool IsTextureActivate(eTextureUnit textureUnit) const;
    bool IsTextureActivate() const;

    // set texture persistent
    void SetPersistent(bool isPersistent);

    bool IsLoadedFromFile() const { return mLoadedFromFile; }
    bool IsPersistent() const { return mPersistent; }
    bool IsTransparent() const { return mTransparent; }

private:
    // internals
    void DestroyTextureObject();

private:
    bool mLoadedFromFile = false; // image was loaded from image file so it can be reloaded
    bool mPersistent = false; // once it was loaded keep alive forever
    bool mTransparent = false; // texture is transparent
};