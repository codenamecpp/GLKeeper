#include "stdafx.h"
#include "TextureManager.h"
#include "DK2AssetLoader.h"

//////////////////////////////////////////////////////////////////////////

static const Point2D TextureAtlasSize = Point2D {2048, 2048};

//////////////////////////////////////////////////////////////////////////

TextureManager gTextureManager;

//////////////////////////////////////////////////////////////////////////

void TextureManager::Initialize()
{
    mInitRenderDataQueue.reserve(16);
    mAtlases.reserve(16);

    // init default textures
    const std::string defaultWhiteTextureName = "White";
    {
        std::unique_ptr<Texture> defaultTexture = std::make_unique<Texture>(defaultWhiteTextureName, eTextureBacking_None);
        defaultTexture->CreateDefaultWhite();
        mDefaultWhite = defaultTexture.get();
        mStandaloneTextures[defaultWhiteTextureName] = std::move(defaultTexture);
    }
    const std::string defaultBlackTextureName = "Black";
    {
        std::unique_ptr<Texture> defaultTexture = std::make_unique<Texture>(defaultBlackTextureName, eTextureBacking_None);
        defaultTexture->CreateDefaultBlack();
        mDefaultBlack = defaultTexture.get();
        mStandaloneTextures[defaultBlackTextureName] = std::move(defaultTexture);
    }

    //GetTexture("NoTexture", eTextureBacking_None);
    //GetTexture("NoTextureName", eTextureBacking_None);
}

void TextureManager::Shutdown()
{
    mStandaloneTextures.clear();
    mInAtlasTextures.clear();

    mAtlases.clear();
    mInitRenderDataQueue.clear();
    mAtlasesList.clear();

    mDefaultWhite = nullptr;
    mDefaultBlack = nullptr;
}

void TextureManager::OnRenderFrame()
{
    if (!mInitRenderDataQueue.empty())
    {
        for (TextureAtlas* roller: mInitRenderDataQueue)
        {
            roller->EnsureRenderDataInited();
        }
        mInitRenderDataQueue.clear();
    }

    for (const auto& roller: mAtlases)
    {
        roller->UpdateRenderData();
    }
}

TextureSourceId TextureManager::GenerateTextureSourceId()
{
    return ++mNextTextureSourceId;
}

Texture* TextureManager::LoadTexture(const std::string& name, eTextureBacking backingMode)
{
    ResourcesMap& resourcesMap = (backingMode == eTextureBacking_Atlas) ? mInAtlasTextures : mStandaloneTextures;

    Texture* textureInstance = nullptr;
    if (!name.empty())
    {
        cxx_assert(resourcesMap.find(name) == resourcesMap.end());

        std::unique_ptr<Texture> newTextureInstance = std::make_unique<Texture>(name, backingMode);
        newTextureInstance->Load();
        if (newTextureInstance->IsLoaded())
        {
            textureInstance = newTextureInstance.get();
            resourcesMap[name] = std::move(newTextureInstance);
        }
    }
    else
    {
        cxx_assert(false);
        textureInstance = mDefaultWhite;
    }
    return textureInstance;
}

Texture* TextureManager::FindTexture(const std::string& name) const
{
    Texture* resultTexture = FindTexture(name, eTextureBacking_Default);
    if (resultTexture == nullptr)
    {
        resultTexture = FindTexture(name, (eTextureBacking_Default != eTextureBacking_Atlas) ? 
            eTextureBacking_Atlas : 
            eTextureBacking_None);
    }
    return resultTexture;
}

Texture* TextureManager::FindTexture(const std::string& name, eTextureBacking backingMode) const
{
    const ResourcesMap& resourcesMap = (backingMode == eTextureBacking_Atlas) ? mInAtlasTextures : mStandaloneTextures;

    auto mapIt = resourcesMap.find(name);
    if (mapIt != resourcesMap.end())
    {
        return mapIt->second.get();
    }
    return nullptr;
}

Texture* TextureManager::GetTexture(const std::string& name, eTextureBacking backingMode)
{
    Texture* textureInstance = FindTexture(name, backingMode);
    if (textureInstance == nullptr)
    {
        textureInstance = LoadTexture(name, backingMode);
    }
    cxx_assert(textureInstance);
    return textureInstance;
}

Texture* TextureManager::GetTexture(const std::string& name)
{
    Texture* textureInstance = FindTexture(name);
    if (textureInstance == nullptr)
    {
        textureInstance = LoadTexture(name, eTextureBacking_Default);
    }
    cxx_assert(textureInstance);
    return textureInstance;
}

bool TextureManager::CreateTextureAtlasEntry(const std::string& textureName, BitmapImage& sourceBitmap, TextureAtlasEntry& outEntry)
{
    if (!sourceBitmap.HasContent())
    {
        cxx_assert(false);
        return false;
    }

    const Point2D imageBitmapDims = sourceBitmap.GetDimensions();
    if ((imageBitmapDims.x > TextureAtlasSize.x) ||
        (imageBitmapDims.y > TextureAtlasSize.y))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Texture '%s' exceeds atlas size limit", textureName.c_str());
        return false;
    }

    // try use exising atlas
    TextureRegion textureRegion {};
    for (TextureAtlas* roller: mAtlasesList)
    {
        if (roller->TryAppendTexture(sourceBitmap, textureRegion))
        {
            outEntry.mTextureAtlas = roller;
            outEntry.mTextureRegion = textureRegion;
            return true;
        }
    }
    const TextureSourceId newAtlasSourceId = GenerateTextureSourceId();
    // create new atlas
    TextureAtlas* newAtlasPtr = mAtlases.emplace_back(std::make_unique<TextureAtlas>(newAtlasSourceId)).get();
    if (!newAtlasPtr->Setup(TextureAtlasSize, sourceBitmap.GetPixelFormat(), sourceBitmap.GetMipsCount()) ||
        !newAtlasPtr->TryAppendTexture(sourceBitmap, textureRegion))
    {
        cxx_assert(false);
        gConsole.LogMessage(eLogLevel_Warning, "Failed to create texture atlas");
        return false;
    }

    outEntry.mTextureAtlas = newAtlasPtr;
    outEntry.mTextureRegion = textureRegion;

    mAtlasesList.push_back(newAtlasPtr);

    mInitRenderDataQueue.push_back(newAtlasPtr);

    return true;
}
