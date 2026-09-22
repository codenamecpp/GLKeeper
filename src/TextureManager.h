#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Texture.h"
#include "TextureAtlas.h"

//////////////////////////////////////////////////////////////////////////

class TextureManager : public cxx::noncopyable
{
    friend class Texture;

public:
    void Initialize();
    void Shutdown();

    void OnRenderFrame();

    // returns already loaded texture (params are ignored) or loads new texture otherwise
    // returns nullptr if name is empty
    Texture* GetTexture(const std::string& name, eTextureBacking backingMode);
    Texture* GetTexture(const std::string& name);
    // returns loaded texture or nullptr if it not found
    Texture* FindTexture(const std::string& name, eTextureBacking backingMode) const;
    Texture* FindTexture(const std::string& name) const;

    inline Texture* GetWhiteTexture() const { return mDefaultWhite; }
    inline Texture* GetBlackTexture() const { return mDefaultBlack; }

private:
    TextureSourceId GenerateTextureSourceId();

    Texture* LoadTexture(const std::string& name, eTextureBacking backingMode);

    // allocates texture entry
    bool CreateTextureAtlasEntry(const std::string& textureName, BitmapImage& sourceBitmap, TextureAtlasEntry& outEntry);

private:
    TextureSourceId mNextTextureSourceId {};

    using ResourcesMap = std::unordered_map<std::string, std::unique_ptr<Texture>, cxx::icase_string_hashfunc, cxx::icase_string_eq>;
    ResourcesMap mStandaloneTextures;
    ResourcesMap mInAtlasTextures;

    Texture* mDefaultWhite = nullptr;
    Texture* mDefaultBlack = nullptr;

    // texture atlases    
    std::vector<std::unique_ptr<TextureAtlas>> mAtlases;
    std::vector<TextureAtlas*> mInitRenderDataQueue;
    std::vector<TextureAtlas*> mAtlasesList;
};

//////////////////////////////////////////////////////////////////////////

extern TextureManager gTextureManager;

//////////////////////////////////////////////////////////////////////////