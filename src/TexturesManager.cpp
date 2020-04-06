#include "pch.h"
#include "TexturesManager.h"
#include "Texture2D.h"
#include "Texture2D_Image.h"
#include "Texture2DAnimation.h"
#include "TimeManager.h"

//////////////////////////////////////////////////////////////////////////

static const float WaterAnimationSpeed = 9.2f;
static const int WaterAnimationFramesCount = 32;

static const float LavaAnimationSpeed = 1.0f;
static const int LavaAnimationFramesCount = 1;

//////////////////////////////////////////////////////////////////////////

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

    mTextureAnimationsList.clear();
}

void TexturesManager::UpdateFrame()
{
    float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();

    // process registered texture animations
    for (Texture2DAnimation* currAnimation: mTextureAnimationsList)
    {
        currAnimation->AdvanceAnimation(deltaTime);
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
    Point colorTextureDims { 1, 1 };
    Point dummyTextureDims { 64, 64 };
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

    mLavaTexture = new Texture2D { "Lava" };
    mLavaTexture->SetPersistent(true);

    mWaterTexture = new Texture2D { "Water" };
    mWaterTexture->SetPersistent(true);

    InitWaterLavaTextureAnimations();
}

void TexturesManager::FreeStandardTextures()
{
    SafeDelete(mWhiteTexture);
    SafeDelete(mBlackTexture);
    SafeDelete(mMissingTexture);
    SafeDelete(mWaterTexture);
    SafeDelete(mLavaTexture);
}

void TexturesManager::FreeTextures()
{
    for (auto& curr: mTextures2DMap)
    {
        delete curr.second;
    }

    mTextures2DMap.clear();
    mWaterTextureAnimation.Clear();
}

void TexturesManager::InitWaterLavaTextureAnimations()
{
    // init water animation
    mWaterTextureAnimation.Clear();
    mWaterTextureAnimation.SetAnimationSpeed(WaterAnimationSpeed);
    mWaterTextureAnimation.SetAnimatingTexture(mWaterTexture);

    for (int iTexture = 0; iTexture < WaterAnimationFramesCount; ++iTexture)
    {
        std::string textureName = cxx::va("Water%d", iTexture);

        Texture2D* animationFrame = GetTexture2D(textureName);
        mWaterTextureAnimation.AddAnimationFrame(animationFrame);
    }

    TextureSamplerState textureSamplerState { eTextureFilterMode_Trilinear, eTextureRepeatMode_Repeat };
    mWaterTextureAnimation.SetFramesSamplerState(textureSamplerState);
    RegisterTextureAnimation(&mWaterTextureAnimation);

    // init lava animation
    mLavaTextureAnimation.Clear();
    mLavaTextureAnimation.SetAnimationSpeed(LavaAnimationSpeed);
    mLavaTextureAnimation.SetAnimatingTexture(mLavaTexture);

    for (int iTexture = 0; iTexture < LavaAnimationFramesCount; ++iTexture)
    {
        std::string textureName = cxx::va("Lava%d", iTexture);

        Texture2D* animationFrame = GetTexture2D(textureName);
        mLavaTextureAnimation.AddAnimationFrame(animationFrame);
    }
    mLavaTextureAnimation.SetFramesSamplerState(textureSamplerState);
    RegisterTextureAnimation(&mLavaTextureAnimation);

}

void TexturesManager::RegisterTextureAnimation(Texture2DAnimation* textureAnimation)
{
    if (!cxx::contains(mTextureAnimationsList, textureAnimation))
    {
        mTextureAnimationsList.push_back(textureAnimation);
        return;
    }

    debug_assert(false);
}

void TexturesManager::UnregisterTextureAnimation(Texture2DAnimation* textureAnimation)
{
    cxx::erase_elements(mTextureAnimationsList, textureAnimation);
}

void TexturesManager::HandleScreenResolutionChanged()
{

}
