#include "pch.h"
#include "TexturesManager.h"
#include "Texture2D.h"
#include "Texture2D_Image.h"
#include "Texture2DAnimation.h"
#include "TimeManager.h"

TexturesManager gTexturesManager;

bool TexturesManager::Initialize()
{
    InitStandardTextures();

    return true;
}

void TexturesManager::Deinit()
{
    FreeStandardTextures();
    FreeTextures();
}

void TexturesManager::UpdateFrame()
{
    float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();

    for (auto& curr: mTexture2DAnimations)
    {
        curr.second->UpdateAnimation(deltaTime);
    }
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

void TexturesManager::InitStandardTextures()
{
    Texture2D_Image textureData;
    Size2D colorTextureDims { 1, 1 };
    Size2D dummyTextureDims { 64, 64 };
    textureData.CreateImage(eTextureFormat_RGBA8, colorTextureDims, 0, false);

    TextureSamplerState textureSamplerState { eTextureFilterMode_Bilinear, eTextureRepeatMode_Repeat };

    mWhiteTexture = new Texture2D { "white" };
    mWhiteTexture->SetPersistent(true);
    mWhiteTexture->SetSamplerState(textureSamplerState);

    textureData.FillWithColor(Color32_White, 0);
    if (!mWhiteTexture->CreateTexture(textureData))
    {
        debug_assert(false);
    }

    mBlackTexture = new Texture2D { "black" };
    mBlackTexture->SetPersistent(true);
    mBlackTexture->SetSamplerState(textureSamplerState);

    textureData.FillWithColor(Color32_Black, 0);
    if (!mBlackTexture->CreateTexture(textureData))
    {
        debug_assert(false);
    }

    mMissingTexture = new Texture2D { "missing" };
    mMissingTexture->SetPersistent(true);
    mMissingTexture->SetSamplerState(textureSamplerState);

    textureData.CreateImage(eTextureFormat_RGBA8, dummyTextureDims, 0, false);
    textureData.FillWithCheckerBoard(0);
    if (!mMissingTexture->CreateTexture(textureData))
    {
        debug_assert(false);
    }

    textureSamplerState.mFilterMode = eTextureFilterMode_Trilinear;

    // init standard game textures
    mLavaTexture = GetTexture2D("Lava0");
    mLavaTexture->SetPersistent(true);
    mLavaTexture->SetSamplerState(textureSamplerState);

    mWaterTexture = GetTexture2D("Water0"); // todo: animate
    mWaterTexture->SetPersistent(true);
    mLavaTexture->SetSamplerState(textureSamplerState);
}

void TexturesManager::FreeStandardTextures()
{
    SafeDelete(mWhiteTexture);
    SafeDelete(mBlackTexture);
    SafeDelete(mMissingTexture);

    // do not delete those
    mLavaTexture = nullptr;
    mWaterTexture = nullptr;
}

void TexturesManager::FreeTextures()
{
    for (auto& curr: mTextures2DMap)
    {
        delete curr.second;
    }

    for (auto& curr: mTexture2DAnimations)
    {
        delete curr.second;
    }

    mTexture2DAnimations.clear();
    mTextures2DMap.clear();
}
