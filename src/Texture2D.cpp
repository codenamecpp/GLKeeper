#include "pch.h"
#include "Texture2D.h"
#include "GpuTexture2D.h"
#include "GraphicsDevice.h"
#include "Texture2D_Image.h"
#include "EngineTexturesProvider.h"
#include "Console.h"

Texture2D::Texture2D(const std::string& textureName)
    : mTextureName(textureName)
    , mSamplerState()
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

    mTextureDesc = Texture2D_Desc();

    // clear loading flags
    mPersistent = false;
    mLoadedFromFile = false;
}

void Texture2D::SetSamplerState(const TextureSamplerState& samplerState)
{
    mSamplerState = samplerState;
    if (mGpuTextureObject)
    {
        mGpuTextureObject->SetSamplerState(samplerState);
    }
}

bool Texture2D::ActivateTexture(eTextureUnit textureUnit)
{
    if (!IsTextureLoaded())
    {
        if (!LoadTexture()) // try to load
        {
            debug_assert(false);
            return false;
        }
    }
    debug_assert(mGpuTextureObject);
    gGraphicsDevice.BindTexture(textureUnit, mGpuTextureObject);
    return true;
}

bool Texture2D::LoadTexture()
{
    if (IsTextureLoaded())
        return true;

    Texture2D_Image imageData;
    if (gEngineTexturesProvider.ContainsTexture(mTextureName))
    {
        if (!gEngineTexturesProvider.ExtractTexture(mTextureName, imageData))
        {
            gConsole.LogMessage(eLogMessage_Debug, "Fail to load texture '%s'", mTextureName.c_str());
            debug_assert(false);
        }
    }
    else
    {
        // todo: load textures from wad
        debug_assert(false);
    }
    
    if (imageData.IsNull()) // create dummy texture
    {
        Size2D dummyTextureDims { 64, 64 };
        imageData.CreateImage(eTextureFormat_RGBA8, dummyTextureDims, 0, false);
        imageData.FillWithCheckerBoard();
        if (!CreateTexture(imageData))
        {
            debug_assert(false);
        }
        return false;
    }

    // force pot dims
    if (imageData.NonPOT())
    {
        imageData.ResizeToPOT();
    }

    mLoadedFromFile = true;
    mTextureDesc = imageData.mTextureDesc;
    if (mGpuTextureObject == nullptr)
    {
        mGpuTextureObject = gGraphicsDevice.CreateTexture2D();
        debug_assert(mGpuTextureObject);
    }

    if (!mGpuTextureObject->InitTextureObject(mTextureDesc, imageData.GetImageDataBuffer()))
    {
        debug_assert(false);
    }
    // upload mipmaps
    for (int imipmap = 1; imipmap < imageData.mTextureDesc.mMipmapsCount + 1; ++imipmap)
    {
        const void* mipmapData = imageData.GetImageDataBuffer(imipmap);
        debug_assert(mipmapData);

        if (!mGpuTextureObject->TexSubImage(imipmap, mipmapData))
        {
            debug_assert(false);
        }
    }

    mGpuTextureObject->SetSamplerState(mSamplerState);
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
}

bool Texture2D::CreateTexture(const Texture2D_Image& imageData)
{
    if (imageData.IsNull())
    {
        debug_assert(false);
        return false;
    }

    // npot textures must be converted to pot manually
    if (imageData.NonPOT())
    {
        debug_assert(false);
        return false;
    }

    if (mGpuTextureObject == nullptr)
    {
        mGpuTextureObject = gGraphicsDevice.CreateTexture2D();
        debug_assert(mGpuTextureObject);
    }

    mTextureDesc = imageData.mTextureDesc;

    if (!mGpuTextureObject->InitTextureObject(mTextureDesc, imageData.GetImageDataBuffer(0)))
    {
        debug_assert(false);
    }

    // upload mipmaps
    for (int imipmap = 1; imipmap < imageData.mTextureDesc.mMipmapsCount + 1; ++imipmap)
    {
        const void* mipmapData = imageData.GetImageDataBuffer(imipmap);
        debug_assert(mipmapData);

        if (!mGpuTextureObject->TexSubImage(imipmap, mipmapData))
        {
            debug_assert(false);
        }
    }

    mGpuTextureObject->SetSamplerState(mSamplerState);

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