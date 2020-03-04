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
    , mSamplerState()
    , mSize()
    , mFormat()
    , mMipmapCount()
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

    debug_assert(cxx::get_next_pot(dimensions.x) == dimensions.x);
    debug_assert(cxx::get_next_pot(dimensions.y) == dimensions.y);

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
    mMipmapCount = 0;

    ScopeBinder scopedBind {this};

    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, mSize.x, mSize.y, 0, formatGL, dataType, sourceData);
    glCheckError();

    // set default filter and repeat mode for texture
    SetSamplerStateImpl();
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
    mSize = textureData.mSize;
    mMipmapCount = textureData.GetMipmapsCount();

    ScopeBinder scopedBind {this};

    if (textureData.HasMipmaps())
    {
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, textureData.GetMipmapsCount());
        glCheckError();
    }

    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, mSize.x, mSize.y, 0, formatGL, dataType, textureData.mBitmap.data());
    glCheckError();

    // upload mipmaps
    for (int iMipmap = 0; iMipmap < mMipmapCount; ++iMipmap)
    {
        ::glTexImage2D(GL_TEXTURE_2D, iMipmap + 1, internalFormatGL, 
            textureData.mMipmaps[iMipmap].mSize.x, 
            textureData.mMipmaps[iMipmap].mSize.y, 0, formatGL, dataType, 
            textureData.mMipmaps[iMipmap].mBitmap.data());
        glCheckError();
    }

    // set default filter and repeat mode for texture
    SetSamplerStateImpl();
    return true;
}

void GpuTexture2D::SetMipmapsCount(int mipmapsCount)
{
    if (mMipmapCount == mipmapsCount)
        return;

    bool forceSetSamplerState = (mipmapsCount == 0) || (mMipmapCount == 0);
    mMipmapCount = mipmapsCount;

    ScopeBinder scopedBind {this};

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mMipmapCount);
    glCheckError();

    if (forceSetSamplerState)
    {
        SetSamplerStateImpl();
    }
}

void GpuTexture2D::SetSamplerState(const TextureSamplerState& samplerState)
{
    if (mSamplerState == samplerState)
        return;

    mSamplerState = samplerState;

    ScopeBinder scopedBind {this};
    SetSamplerStateImpl();
}

bool GpuTexture2D::IsTextureBound(eTextureUnit textureUnit) const
{
    debug_assert(textureUnit < eTextureUnit_COUNT);
    return this == mGraphicsContext.mCurrentTextures[textureUnit].mTexture2D;
}

bool GpuTexture2D::IsTextureBound() const
{
    for (int itexunit = 0; itexunit < eTextureUnit_COUNT; ++itexunit)
    {
        if (this == mGraphicsContext.mCurrentTextures[itexunit].mTexture2D)
            return true;
    }

    return false;
}

bool GpuTexture2D::TexSubImage(int mipLevel, int xoffset, int yoffset, int sizex, int sizey, const void* sourceData)
{
    debug_assert(mipLevel < mMipmapCount);

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

void GpuTexture2D::SetSamplerStateImpl()
{
    // set filtering
    GLint magFilterGL = GL_NEAREST;
    GLint minFilterGL = mMipmapCount > 0 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    switch (mSamplerState.mFilterMode)
    {
        case eTextureFilterMode_Nearest: 
            break;
        case eTextureFilterMode_Bilinear:
            magFilterGL = GL_LINEAR;
            minFilterGL = mMipmapCount > 0 ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
            break;
        case eTextureFilterMode_Trilinear:
            magFilterGL = GL_LINEAR;
            minFilterGL = mMipmapCount > 0 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
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

    // set wrap mode
    GLint gl_wrapS = EnumToGL(mSamplerState.mRepeatModeS);
    GLint gl_wrapT = EnumToGL(mSamplerState.mRepeatModeT);
    GLint gl_wrapR = EnumToGL(mSamplerState.mRepeatModeR);

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_wrapS);
    glCheckError();

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_wrapT);
    glCheckError();

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, gl_wrapR);
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
