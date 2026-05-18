#include "stdafx.h"
#include "TextureManager.h"

//////////////////////////////////////////////////////////////////////////

TextureManager gTextureManager;

//////////////////////////////////////////////////////////////////////////

void TextureManager::Initialize()
{

}

void TextureManager::Shutdown()
{
    mResourcesMap.clear();
}

Texture* TextureManager::LoadTexture(const std::string& name, const LoadParams& params)
{
    if (name.empty())
    {
        cxx_assert(false);
        return nullptr;
    }

    Texture* textureInstance = FindTexture(name);
    cxx_assert(textureInstance == nullptr);
    if (textureInstance)
    {
        gConsole.LogMessage(eLogLevel_Debug, "Texture '%s' is already loaded", name.c_str());
    }
    else
    {
        std::unique_ptr<Texture> newTextureInstance = std::make_unique<Texture>(name);
        newTextureInstance->Load(params.mConvertNPOT);

        bool iSuccess = newTextureInstance->IsLoadedFromFile() || (params.mInitDefaultOnFail && newTextureInstance->IsDefaultTexture());
        if (iSuccess)
        {
            textureInstance = newTextureInstance.get();
            mResourcesMap[name] = std::move(newTextureInstance);
        }
    }
    return textureInstance;
}

Texture* TextureManager::FindTexture(const std::string& name) const
{
    auto mapIt = mResourcesMap.find(name);
    if (mapIt != mResourcesMap.end())
    {
        return mapIt->second.get();
    }
    return nullptr;
}

Texture* TextureManager::GetTexture(const std::string& name, const LoadParams& params)
{
    Texture* textureInstance = FindTexture(name);
    if (textureInstance == nullptr)
    {
        textureInstance = LoadTexture(name, params);
    }
    cxx_assert(textureInstance);
    return textureInstance;
}

Texture* TextureManager::GetMissingTexture()
{
    return GetTexture("NoTexture");
}

Texture* TextureManager::GetMissingTextureWithText()
{
    return GetTexture("NoTextureName");
}

Texture* TextureManager::GetWhiteTexture()
{
    return GetTexture("White");
}
