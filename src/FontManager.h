#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Font.h"

//////////////////////////////////////////////////////////////////////////

class FontManager : public cxx::noncopyable
{
public:
    void Initialize();
    void Shutdown();

    struct LoadParams
    {
        // will create fallback font if loading fails for some reason
        bool mInitDefaultOnFail = true;
    };

    // loads new font if it is not loaded yet
    // will always return true if mInitDefaultOnFail is set to true (operation never fails)
    // will not init render data automatically
    // returns nullptr if name is empty
    Font* LoadFont(const std::string& name, const LoadParams& params = {});

    // returns loaded font or nullptr if it not found
    Font* FindFont(const std::string& name) const;

    // returns already loaded font (params are ignored) or loads new font otherwise
    // returns nullptr if name is empty
    Font* GetFont(const std::string& name, const LoadParams& params = {});

private:
    using ResourcesMap = std::map<std::string, std::unique_ptr<Font>, cxx::icase_string_less>;
    ResourcesMap mResourcesMap;
};

//////////////////////////////////////////////////////////////////////////

extern FontManager gFontManager;

//////////////////////////////////////////////////////////////////////////