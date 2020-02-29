#include "pch.h"
#include "GpuTexture2D.h"
#include "OpenGL_Defs.h"
#include "GraphicsDeviceContext.h"
#include "GraphicsDevice.h"
#include "Texture2D_Data.h"

//////////////////////////////////////////////////////////////////////////

class GpuTexture2D::ScopeBinder
{
public:
    ScopeBinder(GpuTexture2D* gpuTexture)
        : mPreviousTexture(gpuTexture->mGraphicsContext.mCurrentTextures[gpuTexture->mGraphicsContext.mCurrentTextureUnit].mTexture2D)
        , mTexture(gpuTexture)
    {
        debug_assert(gpuTexture);
        if (mTexture != mPreviousTexture)
        {
            ::glBindTexture(GL_TEXTURE_2D, mTexture->mResourceHandle);
            glCheckError();
        }
    }
    ~ScopeBinder()
    {
        if (mTexture != mPreviousTexture)
        {
            ::glBindTexture(GL_TEXTURE_2D, mPreviousTexture ? mPreviousTexture->mResourceHandle : 0);
            glCheckError();
        }
    }
private:
    GpuTexture2D* mPreviousTexture;
    GpuTexture2D* mTexture;
};

//////////////////////////////////////////////////////////////////////////

GpuTexture2D::GpuTexture2D(GraphicsDeviceContext& renderContext)
    : mGraphicsContext(renderContext)
    , mResourceHandle()
    , mFiltering()
    , mRepeating()
    , mSize()
    , mFormat()
    , mMipmapsCount()
{
    ::glGenTextures(1, &mResourceHandle);
    glCheckError();
}

GpuTexture2D::~GpuTexture2D()
{
    SetUnbound();

    ::glDeleteTextures(1, &mResourceHandle);
    glCheckError();
}

bool GpuTexture2D::SetTextureData(eTextureFormat textureFormat, const Size2D& dimensions, const void* sourceData)
{
    if (textureFormat == eTextureFormat_Null)
        return false;

    debug_assert(cxx::is_pot(dimensions.x));
    debug_assert(cxx::is_pot(dimensions.y));

    GLuint formatGL = GetTextureInputFormatGL(textureFormat);
    GLint internalFormatGL = GetTextureInternalFormatGL(textureFormat);
    GLenum dataType = GetTextureDataTypeGL(textureFormat);
    if (formatGL == 0 || internalFormatGL == 0 || dataType == 0)
    {
        debug_assert(false);
        return false;
    }

    mFormat = textureFormat;
    mSize = dimensions;
    mMipmapsCount = 0;

    ScopeBinder scopedBind {this};

    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, mSize.x, mSize.y, 0, formatGL, dataType, sourceData);
    glCheckError();

    // set default filter and repeat mode for texture
    SetSamplerStateImpl(eTextureFilterMode_Nearest, eTextureWrapMode_ClampToEdge);
    return true;
}

bool GpuTexture2D::SetTextureData(const Texture2D_Data& textureData)
{
    if (textureData.IsNull())
    {
        debug_assert(false);
        return false;
    }

    debug_assert(textureData.IsPOT());

    GLuint formatGL = GetTextureInputFormatGL(textureData.mPixelsFormat);
    GLint internalFormatGL = GetTextureInternalFormatGL(textureData.mPixelsFormat);
    GLenum dataType = GetTextureDataTypeGL(textureData.mPixelsFormat);
    if (formatGL == 0 || internalFormatGL == 0 || dataType == 0)
    {
        debug_assert(false);
        return false;
    }

    mFormat = textureData.mPixelsFormat;
    mSize.x = textureData.mDimsW;
    mSize.y = textureData.mDimsH;
    mMipmapsCount = textureData.GetMipmapsCount();

    ScopeBinder scopedBind {this};

    if (textureData.HasMipmaps())
    {
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, textureData.GetMipmapsCount());
        glCheckError();
    }

    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, mSize.x, mSize.y, 0, formatGL, dataType, textureData.mBitmap.data());
    glCheckError();

    // upload mipmaps
    for (int iMipmap = 0; iMipmap < mMipmapsCount; ++iMipmap)
    {
        ::glTexImage2D(GL_TEXTURE_2D, iMipmap + 1, internalFormatGL, 
            textureData.mMipmaps[iMipmap].mDimsW, 
            textureData.mMipmaps[iMipmap].mDimsH, 0, formatGL, dataType, 
            textureData.mMipmaps[iMipmap].mBitmap.data());
        glCheckError();
    }

    // set default filter and repeat mode for texture
    SetSamplerStateImpl(eTextureFilterMode_Nearest, eTextureWrapMode_ClampToEdge);
    return true;
}

void GpuTexture2D::SetMipmapsCount(int mipmapsCount)
{
    if (!IsTextureInited())
    {
        debug_assert(false);
        return;
    }

    if (mMipmapsCount == mipmapsCount)
        return;

    bool forceSetSamplerState = (mipmapsCount == 0) || (mMipmapsCount == 0);
    mMipmapsCount = mipmapsCount;

    ScopeBinder scopedBind {this};

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mMipmapsCount);
    glCheckError();

    if (forceSetSamplerState)
    {
        SetSamplerStateImpl(mFiltering, mRepeating);
    }
}

void GpuTexture2D::SetSamplerState(eTextureFilterMode filtering, eTextureWrapMode repeating)
{
    if (!IsTextureInited())
    {
        debug_assert(false);
        return;
    }

    // already set
    if (mFiltering == filtering && mRepeating == repeating)
        return;

    ScopeBinder scopedBind {this};

    SetSamplerStateImpl(filtering, repeating);
}

bool GpuTexture2D::IsTextureBound(eTextureUnit textureUnit) const
{
    if (!IsTextureInited())
        return false;

    debug_assert(textureUnit < eTextureUnit_COUNT);
    return this == mGraphicsContext.mCurrentTextures[textureUnit].mTexture2D;
}

bool GpuTexture2D::IsTextureBound() const
{
    if (!IsTextureInited())
        return false;

    for (int itexunit = 0; itexunit < eTextureUnit_COUNT; ++itexunit)
    {
        if (this == mGraphicsContext.mCurrentTextures[itexunit].mTexture2D)
            return true;
    }

    return false;
}

bool GpuTexture2D::TexSubImage(int mipLevel, int xoffset, int yoffset, int sizex, int sizey, const void* sourceData)
{
    if (!IsTextureInited())
        return false;

    debug_assert(mipLevel < mMipmapsCount);

    debug_assert(sourceData);
    GLuint formatGL = GetTextureInputFormatGL(mFormat);
    GLenum dataType = GetTextureDataTypeGL(mFormat);
    if (formatGL == 0 || dataType == 0)
    {
        debug_assert(false);
        return false;
    }

    ScopeBinder scopedBind {this};
    ::glTexSubImage2D(GL_TEXTURE_2D, mipLevel, xoffset, yoffset, sizex, sizey, formatGL, dataType, sourceData);
    glCheckError();
    return true;
}

bool GpuTexture2D::TexSubImage(int mipLevel, int sizex, int sizey, const void* sourceData)
{
    return TexSubImage(mipLevel, 0, 0, mSize.x, mSize.y, sourceData);
}

bool GpuTexture2D::IsTextureInited() const
{
    return mFormat != eTextureFormat_Null;
}

void GpuTexture2D::SetSamplerStateImpl(eTextureFilterMode filtering, eTextureWrapMode repeating)
{
    mFiltering = filtering;
    mRepeating = repeating;

    // set filtering
    GLint magFilterGL = GL_NEAREST;
    GLint minFilterGL = mMipmapsCount > 0 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    switch (filtering)
    {
        case eTextureFilterMode_Nearest: 
            break;
        case eTextureFilterMode_Bilinear:
            magFilterGL = GL_LINEAR;
            minFilterGL = mMipmapsCount > 0 ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
            break;
        case eTextureFilterMode_Trilinear:
            magFilterGL = GL_LINEAR;
            minFilterGL = mMipmapsCount > 0 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
            break;
        default:
        {
            debug_assert(false);
        }
        break;
    }

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterGL);
    glCheckError();

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterGL);
    glCheckError();

    // set repeating
    GLint wrapSGL = GL_CLAMP_TO_EDGE;
    GLint wrapTGL = GL_CLAMP_TO_EDGE;
    switch (repeating)
    {
        case eTextureWrapMode_Repeat:
            wrapSGL = GL_REPEAT;
            wrapTGL = GL_REPEAT;
            break;
        case eTextureWrapMode_ClampToEdge:
            break;
        default:
        {
            debug_assert(false);
        }
        break;
    }

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapSGL);
    glCheckError();

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTGL);
    glCheckError();
}

void GpuTexture2D::SetUnbound()
{
    for (int iTextureUnit = 0; iTextureUnit < eTextureUnit_COUNT; ++iTextureUnit)
    {
        if (this == mGraphicsContext.mCurrentTextures[iTextureUnit].mTexture2D)
        {
            mGraphicsContext.mCurrentTextures[iTextureUnit].mTexture2D = nullptr;
        }
    }
}
