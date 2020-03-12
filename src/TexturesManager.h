#pragma once

#include "ResourceDefs.h"

// forwards
class Texture2DAnimation;

// textures manager class
class TexturesManager: public cxx::noncopyable
{
public:
    Texture2D* mWhiteTexture = nullptr;
    Texture2D* mBlackTexture = nullptr;
    Texture2D* mDummyTexture = nullptr;

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

    // find or create texture2d animation
    // @param animationName: Animation name
    Texture2DAnimation* FindTexture2DAnimation(const std::string& animationName) const;
    Texture2DAnimation* GetTexture2DAnimation(const std::string& animationName);

private:
    void InitDefaultTextures();
    void FreeDefaultTextures();
    void FreeTextures();

private:
    using Textures2DMap = std::map<std::string, Texture2D*, cxx::icase_less>;
    Textures2DMap mTextures2DMap;

    using Textures2DAnimsMap = std::map<std::string, Texture2DAnimation*, cxx::icase_less>;
    Textures2DAnimsMap mTexture2DAnimations;
};

extern TexturesManager gTexturesManager;