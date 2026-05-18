#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Texture.h"

//////////////////////////////////////////////////////////////////////////

class TextureManager : public cxx::noncopyable
{
public:
    void Initialize();
    void Shutdown();

    struct LoadParams
    {
        // will convert non-power-of-two textures
        bool mConvertNPOT = false;
        // will create dummy texture if loading fails for some reason
        bool mInitDefaultOnFail = true;
    };

    // loads new texture if it is not loaded yet
    // will always return true if mInitDefaultOnFail is set to true (operation never fails)
    // will not init render data automatically
    // returns nullptr if name is empty
    Texture* LoadTexture(const std::string& name, const LoadParams& params = {});

    // returns loaded texture or nullptr if it not found
    Texture* FindTexture(const std::string& name) const;

    // returns already loaded texture (params are ignored) or loads new texture otherwise
    // returns nullptr if name is empty
    Texture* GetTexture(const std::string& name, const LoadParams& params = {});

    // base textures
    Texture* GetMissingTexture();
    Texture* GetMissingTextureWithText();
    Texture* GetWhiteTexture();

private:
    using ResourcesMap = std::map<std::string, std::unique_ptr<Texture>, cxx::icase_string_less>;
    ResourcesMap mResourcesMap;
};

//////////////////////////////////////////////////////////////////////////

extern TextureManager gTextureManager;

//////////////////////////////////////////////////////////////////////////