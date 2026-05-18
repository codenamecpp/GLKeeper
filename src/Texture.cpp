#include "stdafx.h"
#include "Texture.h"
#include "DK2AssetLoader.h"

Texture::Texture(const std::string& textureName)
    : mTextureName(textureName)
{
}

void Texture::Load(bool convertNPOT)
{
    Purge();

    if (LoadFromFile(convertNPOT))
    {
        mIsLoadedFromFile = true;
    }
    else
    {
        InitDefault();
    }

    mHasAlpha = mImageBitmap.GetHasAlphaHint();
    mPixelFormat = mImageBitmap.GetPixelFormat();
}

void Texture::InitDefault()
{
    Purge();

    mIsDefaultTexture = true;

    const Point2D imageDims {8, 8};
    if (!mImageBitmap.Create(ePixelFormat_RGBA8, imageDims, COLOR_PINK))
    {
        cxx_assert(false);
    }

    mTextureDimensions = imageDims;
    mImageDimensions = imageDims;

    mHasAlpha = mImageBitmap.GetHasAlphaHint();
    mPixelFormat = mImageBitmap.GetPixelFormat();
}

void Texture::Purge()
{
    mImageBitmap.Clear();

    mGpuTextureResource.reset();
    mTextureDimensions = {};
    mImageDimensions = {};

    mIsRenderDataInited = false;
    mIsDefaultTexture = false;
    mIsLoadedFromFile = false;
    mHasMipmaps = false;
    mWasResized = false;
    mHasAlpha = false;

    mPixelFormat = ePixelFormat_Null;
}

void Texture::BindTexture(eTextureUnit textureUnit)
{
    if (!IsRenderDataInited())
    {
        InitRenderData();
    }
    gRenderDevice.BindTexture2D(textureUnit, GetGpuTexturePtr());
}

void Texture::InitRenderData()
{
    if (IsRenderDataInited())
        return;

    if (!IsInited())
    {
        cxx_assert(false);
        return;
    }

    mIsRenderDataInited = true;

    static bool freeSystemBits = true;

    mGpuTextureResource = gRenderDevice.CreateTexture2D(mImageBitmap, eTextureFiltering_Trilinear);
    cxx_assert(mGpuTextureResource);

    if (mGpuTextureResource)
    {
        // success
        if (freeSystemBits)
        {
            mImageBitmap.Clear();
        }

        return;
    }

    cxx_assert(false);

    gConsole.LogMessage(eLogLevel_Warning, "Cannot initialize texture render data ('%s'), trying fallback", mTextureName.c_str());

    // fallback in case of failure
    BitmapImage bi;
    if (!bi.Create(ePixelFormat_RGBA8, Point2D { 8, 8 }, COLOR_PINK))
    {
        cxx_assert(false);
    }

    mGpuTextureResource = gRenderDevice.CreateTexture2D(bi);
    cxx_assert(!!mGpuTextureResource);
}

bool Texture::LoadFromFile(bool convertNPOT)
{
    const std::string& textureName = mTextureName;

    if (textureName.empty())
    {
        cxx_assert(false);
        return false;
    }

    bool isSuccess = false;

    // external file
    std::string assetPath;
    if (gFiles.PathToFile(textureName, assetPath))
    {
        isSuccess = mImageBitmap.LoadFromFile(assetPath);
        if (!isSuccess)
        {
            gConsole.LogMessage(eLogLevel_Warning, "Fail to read image data '%s' from file '%s'", textureName.c_str(), assetPath.c_str());
        }
    }
    else // archived file
    {
        isSuccess = gDK2AssetLoader.LoadImageData(textureName, mImageBitmap);
        if (!isSuccess)
        {
            gConsole.LogMessage(eLogLevel_Warning, "Fail to load image data '%s'", textureName.c_str());
        }
    }

    cxx_assert(isSuccess);

    if (isSuccess)
    {
        mImageDimensions = mImageBitmap.GetDimensions();
        mTextureDimensions = mImageBitmap.GetDimensions();
        mHasMipmaps = mImageBitmap.GetMipsCount() > 1;
        if (convertNPOT && !mImageBitmap.IsPOT())
        {
            isSuccess = ConvertNPOT();
            cxx_assert(isSuccess && mImageBitmap.IsPOT());
        }
    }
    return isSuccess;
}

bool Texture::ConvertNPOT()
{
    bool isSuccess = mImageBitmap.ResizeToPowerOfTwo();
    cxx_assert(isSuccess);

    if (isSuccess)
    {
        mTextureDimensions = mImageBitmap.GetDimensions();
        mWasResized = (mImageDimensions != mTextureDimensions);
    }
    return isSuccess;
}
