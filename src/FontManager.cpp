#include "stdafx.h"
#include "FontManager.h"

//////////////////////////////////////////////////////////////////////////

FontManager gFontManager;

//////////////////////////////////////////////////////////////////////////

void FontManager::Initialize()
{

}

void FontManager::Shutdown()
{
    mResourcesMap.clear();
}

Font* FontManager::LoadFont(const std::string& name, const LoadParams& params)
{
    if (name.empty())
    {
        cxx_assert(false);
        return nullptr;
    }

    Font* fontInstance = FindFont(name);
    cxx_assert(fontInstance == nullptr);
    if (fontInstance)
    {
        gConsole.LogMessage(eLogLevel_Debug, "Font '%s' is already loaded", name.c_str());
    }
    else
    {
        std::unique_ptr<Font> newFontInstance = std::make_unique<Font>(name);
        newFontInstance->Load();

        bool isSuccess = newFontInstance->IsLoadedFromFile() || (params.mInitDefaultOnFail && newFontInstance->IsDefaultFont());
        if (isSuccess)
        {
            fontInstance = newFontInstance.get();
            mResourcesMap[name] = std::move(newFontInstance);
        }
    }
    return fontInstance;
}

Font* FontManager::FindFont(const std::string& name) const
{
    auto mapIt = mResourcesMap.find(name);
    if (mapIt != mResourcesMap.end())
    {
        return mapIt->second.get();
    }
    return nullptr;
}

Font* FontManager::GetFont(const std::string& name, const LoadParams& params)
{
    Font* fontInstance = FindFont(name);
    if (fontInstance == nullptr)
    {
        fontInstance = LoadFont(name, params);
    }
    cxx_assert(fontInstance);
    return fontInstance;
}
