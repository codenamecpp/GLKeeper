#pragma once

class Texture2D;

// render textures manager class
class TexturesManager: public cxx::noncopyable
{
public:
    Texture2D* mWhiteTexture = nullptr;
    Texture2D* mBlackTexture = nullptr;
    Texture2D* mDummyTexture = nullptr;

public:

    // setup texture manager internal resources, returns false on error
    bool Initialize();
    void Deinit();

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

private:
    void InitDefaultTextures();
    void FreeDefaultTextures();
    void FreeTextures();

private:
    using Textures2DMap = std::map<std::string, Texture2D*, cxx::icase_less>;
    Textures2DMap mTextures2DMap;
};

extern TexturesManager gTexturesManager;