#include "pch.h"
#include "Texture2D.h"
#include "GpuTexture2D.h"
#include "GraphicsDevice.h"
#include "Texture2D_Data.h"
#include "EngineTexturesProvider.h"
#include "Console.h"

Texture2D::Texture2D(const std::string& textureName)
    : mTextureName(textureName)
    , mRepeatMode(eTextureRepeatMode_ClampToEdge)
    , mFilterMode(eTextureFilterMode_Nearest)
{
}

Texture2D::~Texture2D()
{
    DestroyTextureObject();
}

void Texture2D::DestroyTextureObject()
{
    if (mGpuTextureObject)
    {
        gGraphicsDevice.DestroyTexture(mGpuTextureObject);
        mGpuTextureObject = nullptr;
    }
    mPersistent = false;
    mLoadedFromFile = false;
}

void Texture2D::SetFilterMode(eTextureFilterMode filterMode)
{
    if (mFilterMode == filterMode)
        return;

    mFilterMode = filterMode;
    if (mGpuTextureObject)
    {
        mGpuTextureObject->SetSamplerState(mFilterMode, mRepeatMode);
    }
}

void Texture2D::SetRepeatMode(eTextureRepeatMode repeatMode)
{
    if (mRepeatMode == repeatMode)
        return;

    mRepeatMode = repeatMode;
    if (mGpuTextureObject)
    {
        mGpuTextureObject->SetSamplerState(mFilterMode, mRepeatMode);
    }
}

bool Texture2D::LoadTexture()
{
    if (IsTextureLoaded())
        return true;

    Texture2D_Data textureData;
    if (gEngineTexturesProvider.ContainsTexture(mTextureName))
    {
        if (!gEngineTexturesProvider.ExtractTexture(mTextureName, textureData))
        {
            gConsole.LogMessage(eLogMessage_Debug, "Fail to load texture '%s'", mTextureName.c_str());
        }
    }
    else
    {
        // todo: load textures from wad
    }

    if (textureData.IsNull()) // create dummy texture
    {
        Size2D dummyTextureDims { 64, 64 };
        textureData.Setup(eTextureFormat_RGBA8, dummyTextureDims, false, nullptr);
        textureData.FillWithCheckerBoard();
        if (!CreateTexture(textureData))
        {
            debug_assert(false);
        }
        return false;
    }

    // force pot dims
    if (textureData.NonPOT())
    {
        textureData.ResizeToPOT(false);
    }

    mLoadedFromFile = true;
    if (mGpuTextureObject == nullptr)
    {
        mGpuTextureObject = gGraphicsDevice.CreateTexture2D();
        debug_assert(mGpuTextureObject);
    }

    if (!mGpuTextureObject->SetTextureData(textureData))
    {
        debug_assert(false);
    }

    mGpuTextureObject->SetSamplerState(mFilterMode, mRepeatMode);
    return true;
}

void Texture2D::FreeTexture()
{
    if (IsTextureLoaded())
    {
        DestroyTextureObject();
    }
    // reset values
    mPersistent     = false;
    mLoadedFromFile = false;
    mTransparent    = false;
}

bool Texture2D::CreateTexture(const Texture2D_Data& textureData)
{
    if (textureData.IsNull())
    {
        debug_assert(false);
        return false;
    }

    // npot textures must be converted to pot manually
    if (textureData.NonPOT())
    {
        debug_assert(false);
        return false;
    }

    if (mGpuTextureObject == nullptr)
    {
        mGpuTextureObject = gGraphicsDevice.CreateTexture2D();
        debug_assert(mGpuTextureObject);
    }

    if (!mGpuTextureObject->SetTextureData(textureData))
    {
        debug_assert(false);
    }

    mGpuTextureObject->SetSamplerState(mFilterMode, mRepeatMode);

    mTransparent = textureData.mTransparent;
    mLoadedFromFile = false;
    return true;
}

bool Texture2D::IsTextureLoaded() const
{
    return mGpuTextureObject != nullptr;
}

bool Texture2D::IsTextureActivate(eTextureUnit textureUnit) const
{
    if (mGpuTextureObject)
    {
        return mGpuTextureObject->IsTextureBound(textureUnit);
    }

    return false;
}

bool Texture2D::IsTextureActivate() const
{
    if (mGpuTextureObject)
    {
        return mGpuTextureObject->IsTextureBound();
    }

    return false;
}

void Texture2D::SetPersistent(bool isPersistent)
{
    mPersistent = isPersistent;
}