#include "stdafx.h"
#include "Texture.h"
#include "DK2AssetLoader.h"
#include "TextureManager.h"

Texture::Texture(const std::string& textureName, eTextureBacking textureBacking)
    : mTextureName(textureName)
    , mTextureBacking(textureBacking)
{
}

void Texture::Load()
{
    if (IsLoaded())
    {
        cxx_assert(false);
        return;
    }

    Purge();

    if (LoadFromFile())
    {
        mIsLoaded = true;
        PostLoad();
    }
    else
    {
        CreateDefault();
    }    
}

void Texture::CreateDefaultWhite()
{
    Purge();

    const Point2D imageDims {8, 8};
    if (!mImageBitmap.Create(ePixelFormat_RGBA8, imageDims, COLOR_WHITE))
    {
        cxx_assert(false);
    }
    mIsDefaultTexture = true;
    PostLoad();
}

void Texture::CreateDefaultBlack()
{
    Purge();

    const Point2D imageDims {8, 8};
    if (!mImageBitmap.Create(ePixelFormat_RGBA8, imageDims, COLOR_BLACK))
    {
        cxx_assert(false);
    }
    mIsDefaultTexture = true;
    PostLoad();
}

void Texture::CreateDefault()
{
    Purge();

    const Point2D imageDims {8, 8};
    if (!mImageBitmap.Create(ePixelFormat_RGBA8, imageDims, COLOR_PINK))
    {
        cxx_assert(false);
    }

    mIsDefaultTexture = true;
    PostLoad();
}

void Texture::PostLoad()
{
    cxx_assert(mImageBitmap.HasContent());

    const Point2D imageDimsBeforeConvert = mImageBitmap.GetDimensions();

    bool isAtlasTexture = (mTextureBacking == eTextureBacking_Atlas);
    bool isConvertNPOTs = !isAtlasTexture && !mImageBitmap.IsPOT();
    if (isConvertNPOTs)
    {
        gConsole.LogMessage(eLogLevel_Info, "Auto convert NPOT texture '%s'", mTextureName.c_str());
        bool isSuccess = ConvertNPOT();
        cxx_assert(isSuccess && mImageBitmap.IsPOT());
    }

    // store metadata 
    mPixelFormat = mImageBitmap.GetPixelFormat();
    mHasAlpha    = mImageBitmap.GetHasAlphaHint();
    mHasMipmaps  = mImageBitmap.GetMipsCount() > 1;

    // backing mode: atlas
    if (isAtlasTexture)
    {
        TextureAtlasEntry textureAtlasEntry {};
        if (!gTextureManager.CreateTextureAtlasEntry(mTextureName, mImageBitmap, textureAtlasEntry))
        {
            gConsole.LogMessage(eLogLevel_Warning, "Failed to create texture atlas extry for '%s'", mTextureName.c_str());
            cxx_assert(false);
        }

        mTextureAtlas = textureAtlasEntry.mTextureAtlas;
        cxx_assert(mTextureAtlas);
        mTextureRegion = textureAtlasEntry.mTextureRegion;

        mGpuTextureResourceReference = mTextureAtlas->GetRenderResource(); // nullptr at this point is ok
        mTextureSourceId = mTextureAtlas->GetTextureSourceId();
    }
    else
    {
        mTextureRegion.Init(Rect2D{0, 0, imageDimsBeforeConvert.x, imageDimsBeforeConvert.y}, mImageBitmap.GetDimensions());
        mTextureSourceId = gTextureManager.GenerateTextureSourceId();
    }
}

void Texture::Purge()
{
    mImageBitmap.Clear();

    mOwnedGpuTextureResource.reset();
    mGpuTextureResourceReference = {};
    mTextureRegion = {};
    mTextureAtlas = {};
    mTextureSourceId = {};
    mPixelFormat = {};
    mIsRenderDataInited = {};
    mIsDefaultTexture = {};
    mIsLoaded = {};
    mHasMipmaps = {};
    mHasAlpha = {};
}

void Texture::BindTexture(eTextureUnit textureUnit)
{
    if (!IsRenderDataInited())
    {
        InitRenderData();
    }

    if (mTextureAtlas)
    {
        mTextureAtlas->BindTexture(textureUnit);
        return;
    }

    gRenderDevice.BindTexture2D(textureUnit, GetGpuTexturePtr());
}

void Texture::InitRenderData()
{
    if (IsRenderDataInited())
        return;

    if (!IsLoaded())
    {
        cxx_assert(false);
        return;
    }

    mIsRenderDataInited = true;

    if (mTextureAtlas)
    {
        cxx_assert(mTextureAtlas);
        mTextureAtlas->EnsureRenderDataInited();
        mGpuTextureResourceReference = mTextureAtlas->GetRenderResource();
    }
    else
    {
        mOwnedGpuTextureResource = gRenderDevice.CreateTexture2D();
        cxx_assert(mOwnedGpuTextureResource);
        mGpuTextureResourceReference = mOwnedGpuTextureResource.get();

        if (!mGpuTextureResourceReference->Create(mImageBitmap, eTextureFiltering_Trilinear))
        {
            cxx_assert(false);
        }
    }

    static bool freeSystemBits = true;
    if (freeSystemBits)
    {
        mImageBitmap.Clear();
    }

    cxx_assert(mGpuTextureResourceReference);
    if (mGpuTextureResourceReference == nullptr)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Falied to initialize texture render data ('%s')", mTextureName.c_str());
    }
}

bool Texture::LoadFromFile()
{
    if (mTextureName.empty())
    {
        cxx_assert(false);
        return false;
    }

    bool isSuccess = false;

    // external file
    std::string assetPath;
    if (gFiles.PathToFile(mTextureName, assetPath))
    {
        isSuccess = mImageBitmap.LoadFromFile(assetPath);
        if (!isSuccess)
        {
            gConsole.LogMessage(eLogLevel_Warning, "Failed to read image data '%s' from file '%s'", mTextureName.c_str(), assetPath.c_str());
        }
    }
    else // archived file
    {
        isSuccess = gDK2AssetLoader.LoadImageData(mTextureName, mImageBitmap);
        if (!isSuccess)
        {
            gConsole.LogMessage(eLogLevel_Warning, "Failed to load image data '%s'", mTextureName.c_str());
        }
    }

    cxx_assert(isSuccess);
    return isSuccess;
}

bool Texture::ConvertNPOT()
{
    bool isSuccess = mImageBitmap.ResizeToPowerOfTwo();
    cxx_assert(isSuccess);
    return isSuccess;
}
