#pragma once

#include "ResourceDefs.h"
#include "Texture2DAnimation.h"

// textures manager class
class TexturesManager: public cxx::noncopyable
{
public:
    // placeholders
    Texture2D* mWhiteTexture = nullptr;
    Texture2D* mBlackTexture = nullptr;
    Texture2D* mMissingTexture = nullptr;

    // standard game textures
    Texture2D* mLavaTexture = nullptr;
    Texture2D* mWaterTexture = nullptr;

public:

    // setup manager internal resources, returns false on error
    bool Initialize();
    void Deinit();

    // process animating textures
    void UpdateFrame();

    // reload unloaded textures to gpu memory
    void ReloadAllTextures();

    // unload textures from gpu memory, does not affect on textures marked as persistent
    void PurgeLoadedTextures();

    // find texture within cache
    // @param textureName: Texture name
    Texture2D* FindTexture2D(const std::string& textureName) const;

    // find texture within cache or create new with specified name
    // does not perform actual texture data loading
    // @param textureName: Texture name
    Texture2D* GetTexture2D(const std::string& textureName);

    // forces texture data loading
    // @param textureName: Texture name
    Texture2D* LoadTexture2D(const std::string& textureName);

    // add or remove texture animation to manager, it will receive updates
    void RegisterTextureAnimation(Texture2DAnimation* textureAnimation);
    void UnregisterTextureAnimation(Texture2DAnimation* textureAnimation);

private:
    void InitStandardTextures();
    void FreeStandardTextures();
    void FreeTextures();

    void InitWaterLavaTextureAnimations();

private:
    using Textures2DMap = std::map<std::string, Texture2D*, cxx::icase_less>;
    Textures2DMap mTextures2DMap;

    std::vector<Texture2DAnimation*> mTextureAnimationsList;

    Texture2DAnimation mWaterTextureAnimation;
    Texture2DAnimation mLavaTextureAnimation;
};

extern TexturesManager gTexturesManager;