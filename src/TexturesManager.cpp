#include "pch.h"
#include "TexturesManager.h"
#include "Texture2D.h"
#include "Texture2D_Data.h"

TexturesManager gTexturesManager;

bool TexturesManager::Initialize()
{
    InitDefaultTextures();

    return true;
}

void TexturesManager::Deinit()
{
    FreeDefaultTextures();
    FreeTextures();
}

Texture2D* TexturesManager::FindTexture2D(const std::string& textureName) const
{
    auto texture_iterator = mTextures2DMap.find(textureName);
    if (texture_iterator != mTextures2DMap.end())
    {
        return texture_iterator->second;
    }

    return nullptr;
}

Texture2D* TexturesManager::GetTexture2D(const std::string& textureName)
{
    auto texture_iterator = mTextures2DMap.find(textureName);
    if (texture_iterator != mTextures2DMap.end())
    {
        return texture_iterator->second;
    }
    // add to cache
    Texture2D* newTexture = new Texture2D { textureName };
    mTextures2DMap[textureName] = newTexture;
    return newTexture;
}

Texture2D* TexturesManager::LoadTexture2D(const std::string& textureName)
{
    Texture2D* texture2D = GetTexture2D(textureName);
    if (!texture2D->IsTextureLoaded())
    {
        if (!texture2D->LoadTexture())
        {
            debug_assert(false);
        }
    }
    return texture2D;
}

void TexturesManager::ReloadAllTextures()
{
    for (auto& curr: mTextures2DMap)
    {
        if (curr.second->IsTextureLoaded())
            continue;

        if (!curr.second->LoadTexture())
        {
            debug_assert(false);
        }
    }
}

void TexturesManager::PurgeLoadedTextures()
{
    for (auto& curr: mTextures2DMap)
    {
        if (curr.second->IsPersistent() || !curr.second->IsTextureLoaded())
            continue;

        curr.second->FreeTexture();
    }
}

void TexturesManager::InitDefaultTextures()
{
    Texture2D_Data textureData;
    Size2D colorTextureDims { 1, 1 };
    Size2D dummyTextureDims { 64, 64 };
    textureData.Setup(eTextureFormat_RGBA8, colorTextureDims, false, nullptr);

    TextureSamplerState textureSamplerState { eTextureFilterMode_Bilinear, eTextureRepeatMode_Repeat };

    mWhiteTexture = new Texture2D { "white" };
    mWhiteTexture->SetPersistent(true);
    mWhiteTexture->SetSamplerState(textureSamplerState);

    textureData.FillWithColor(Color32_White);
    if (!mWhiteTexture->CreateTexture(textureData))
    {
        debug_assert(false);
    }

    mBlackTexture = new Texture2D { "black" };
    mBlackTexture->SetPersistent(true);
    mBlackTexture->SetSamplerState(textureSamplerState);

    textureData.FillWithColor(Color32_Black);
    if (!mBlackTexture->CreateTexture(textureData))
    {
        debug_assert(false);
    }

    mDummyTexture = new Texture2D { "dummy" };
    mDummyTexture->SetPersistent(true);
    mDummyTexture->SetSamplerState(textureSamplerState);

    textureData.Setup(eTextureFormat_RGBA8, dummyTextureDims, false, nullptr);
    textureData.FillWithCheckerBoard();
    if (!mDummyTexture->CreateTexture(textureData))
    {
        debug_assert(false);
    }
}

void TexturesManager::FreeDefaultTextures()
{
    SafeDelete(mWhiteTexture);
    SafeDelete(mBlackTexture);
    SafeDelete(mDummyTexture);
}

void TexturesManager::FreeTextures()
{
    for (auto& curr: mTextures2DMap)
    {
        delete curr.second;
    }

    mTextures2DMap.clear();
}
