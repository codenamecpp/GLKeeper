#include "stdafx.h"
#include "GpuTexture2D.h"
#include "OpenGLDefs.h"

//////////////////////////////////////////////////////////////////////////

eTextureUnit GpuTexture2D::sCurrentTextureUnit = {};
GpuTexture2D* GpuTexture2D::sCurrentTextures2D[eTextureUnit_COUNT] = {};

//////////////////////////////////////////////////////////////////////////

GpuTexture2D::ScopedBinder::ScopedBinder(GpuTexture2D* texture)
    : mTexture(texture)
{
    mPreviousTexture = sCurrentTextures2D[sCurrentTextureUnit];
    if (mTexture != mPreviousTexture)
    {
        ::glBindTexture(GL_TEXTURE_2D, mTexture->mResourceHandle);
        glCheckErrors();
    }
}
GpuTexture2D::ScopedBinder::~ScopedBinder()
{
    if (mTexture != mPreviousTexture)
    {
        ::glBindTexture(GL_TEXTURE_2D, mPreviousTexture ? mPreviousTexture->mResourceHandle : 0);
        glCheckErrors();
    }
}

//////////////////////////////////////////////////////////////////////////

GpuTexture2D::GpuTexture2D()
    : mResourceHandle()
    , mFiltering()
    , mRepeating()
    , mDimensions()
    , mHasMipmaps()
    , mPixelFormat()
{
    ::glGenTextures(1, &mResourceHandle);
    glCheckErrors();
}

GpuTexture2D::~GpuTexture2D()
{
    // set unbound
    for (int iTextureUnit = 0; iTextureUnit < eTextureUnit_COUNT; ++iTextureUnit)
    {
        if (this == sCurrentTextures2D[iTextureUnit])
        {
            sCurrentTextures2D[iTextureUnit] = nullptr;
        }
    }
    ::glDeleteTextures(1, &mResourceHandle);
    glCheckErrors();
}

bool GpuTexture2D::Create(const BitmapImage& picture, eTextureFiltering filter, eTextureRepeating repeat)
{
    cxx_assert(picture.HasContent() && picture.IsPOT());

    bool isSuccess = false;
    if (picture.HasContent() && picture.IsPOT())
    {
        cxx::static_vector<Texture2DMip, 12> textureMips;
        for (int imipmap = 0; imipmap < picture.GetMipsCount(); ++imipmap)
        {
            const Point2D mipDims = picture.GetDimensions(imipmap);

            Texture2DMip currentMip;
            currentMip.mSizex = mipDims.x;
            currentMip.mSizey = mipDims.y;
            currentMip.mPixelsData = picture.GetMipPixels(imipmap);
            textureMips.push_back(currentMip);
        }

        isSuccess = Create(textureMips.size(), textureMips.data(), picture.GetPixelFormat(), filter, repeat);
    }
    return isSuccess;
}

bool GpuTexture2D::Create(int numMipmaps, const Texture2DMip* mipmaps, ePixelFormat pixelFormat, eTextureFiltering filter, eTextureRepeating repeat)
{
    if ((numMipmaps < 1) || (mipmaps == nullptr) || (pixelFormat == ePixelFormat_Null))
    {
        cxx_assert(false);
        return false;
    }

    // validate mipmaps
    
    for (int imipmap = 0; imipmap < numMipmaps; ++imipmap)
    {
        const Texture2DMip& currentMip = mipmaps[imipmap];
        if ((currentMip.mSizex < 1) || !cxx::is_pot(currentMip.mSizex) ||
            (currentMip.mSizey < 1) || !cxx::is_pot(currentMip.mSizey))
        {
            cxx_assert(false);
            return false;
        }
    }

    GLuint formatGL = GetTextureInputFormatGL(pixelFormat);
    GLint internalFormatGL = GetTextureInternalFormatGL(pixelFormat);
    GLenum dataType = GetTextureDataTypeGL(pixelFormat);
    if (formatGL == 0 || internalFormatGL == 0 || dataType == 0)
    {
        cxx_assert(false);
        return false;
    }

    mPixelFormat = pixelFormat;
    mDimensions.x = mipmaps[0].mSizex;
    mDimensions.y = mipmaps[0].mSizey;
    mHasMipmaps = (numMipmaps > 1);

    ScopedBinder scopedBinder(this);

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
    glCheckErrors();

    for (int imipmap = 0; imipmap < numMipmaps; ++imipmap)
    {
        ::glTexImage2D(GL_TEXTURE_2D, imipmap, internalFormatGL, 
            mipmaps[imipmap].mSizex, 
            mipmaps[imipmap].mSizey, 
            0, 
            formatGL, dataType, mipmaps[imipmap].mPixelsData);
        glCheckErrors();
    }

    SetParams(filter, repeat);
    return true;
}

bool GpuTexture2D::Create(const Point2D& dimensions, ePixelFormat pixelFormat, const void* pixeldata, eTextureFiltering filter, eTextureRepeating repeat)
{
    cxx_assert((dimensions.x > 0) && cxx::is_pot(dimensions.x));
    cxx_assert((dimensions.y > 0) && cxx::is_pot(dimensions.y));
    cxx_assert(pixelFormat != ePixelFormat_Null);

    bool isSuccess = false;
    if (pixelFormat != ePixelFormat_Null)
    {
        Texture2DMip currentMip;
        currentMip.mSizex = dimensions.x;
        currentMip.mSizey = dimensions.y;
        currentMip.mPixelsData = pixeldata;
        isSuccess = Create(1, &currentMip, pixelFormat, filter, repeat);
    }
    return true;
}

void GpuTexture2D::SetSamplerState(eTextureFiltering filtering, eTextureRepeating repeating)
{
    ScopedBinder scopedBinder(this);
    SetParams(filtering, repeating);
}

void GpuTexture2D::SetParams(eTextureFiltering filtering, eTextureRepeating repeating)
{
    mFiltering = filtering;
    mRepeating = repeating;

    // set filtering
    GLint magFilterGL = GL_NEAREST;
    GLint minFilterGL = GL_NEAREST;
    switch (filtering)
    {
        case eTextureFiltering_None: break;
        case eTextureFiltering_Bilinear:
        {
            magFilterGL = GL_LINEAR;
            minFilterGL = mHasMipmaps ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
        }
        break;
        case eTextureFiltering_Trilinear:
        {
            magFilterGL = GL_LINEAR;
            minFilterGL = mHasMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        }
        break;

        default:
            cxx_assert(filtering == eTextureFiltering_None);
        break;
    }

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterGL);
    glCheckErrors();

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterGL);
    glCheckErrors();

    // set repeating
    GLint wrapSGL = GL_CLAMP_TO_EDGE;
    GLint wrapTGL = GL_CLAMP_TO_EDGE;
    switch (repeating)
    {
        case eTextureRepeating_ClampToEdge: break;
        case eTextureRepeating_Repeat:
        {
            wrapSGL = GL_REPEAT;
            wrapTGL = GL_REPEAT;
        }
        break;

        default:
            cxx_assert(repeating == eTextureRepeating_Repeat);
        break;
    }

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapSGL);
    glCheckErrors();

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTGL);
    glCheckErrors();
}

bool GpuTexture2D::IsBound(eTextureUnit textureUnit) const
{
    cxx_assert(textureUnit < eTextureUnit_COUNT);
    return sCurrentTextures2D[textureUnit] == this;
}

void GpuTexture2D::Upload(const Rect2D& textureArea, const void* sourceData, int mipLevel)
{
    Upload(textureArea.GetPosition(), textureArea.GetSize(), sourceData, mipLevel);
}

void GpuTexture2D::Upload(const Point2D& offset, const Point2D& dimensions, const void* sourceData, int mipLevel)
{
    cxx_assert(mipLevel >= 0);
    cxx_assert((offset.x >= 0) && (offset.y >= 0));
    cxx_assert((dimensions.x > 0) && (dimensions.y > 0));

    const GLuint formatGL = GetTextureInputFormatGL(mPixelFormat);
    const GLenum dataType = GetTextureDataTypeGL(mPixelFormat);

    ScopedBinder scopedBinder(this);
    ::glTexSubImage2D(GL_TEXTURE_2D, mipLevel, offset.x, offset.y,
        dimensions.x, 
        dimensions.y, 
        formatGL, dataType, sourceData);
    glCheckErrors();
}

void GpuTexture2D::Upload(const void* sourceData, int mipLevel)
{
    static const Point2D zeroOffset {0, 0};

    Upload(zeroOffset, mDimensions, sourceData, mipLevel);
}
