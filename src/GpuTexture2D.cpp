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

bool GpuTexture2D::Create(ePixelFormat textureFormat, const Point2D& dimensions, const void* sourceData)
{
    cxx_assert(dimensions.x > 0);
    cxx_assert(dimensions.y > 0);
    cxx_assert(cxx::is_pot(dimensions.x) && cxx::is_pot(dimensions.y));

    const GLint internalFormatGL = GetTextureInternalFormatGL(textureFormat);
    const GLuint formatGL = GetTextureInputFormatGL(textureFormat);
    const GLenum dataType = GetTextureDataTypeGL(textureFormat);
    if ((formatGL == 0) || (internalFormatGL == 0) || (dataType == 0))
    {
        cxx_assert(false);
        return false;
    }

    mPixelFormat = textureFormat;
    mDimensions = dimensions;
    mHasMipmaps = false;
    
    ScopedBinder scopedBinder(this);
    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, mDimensions.x, mDimensions.y, 0, formatGL, dataType, sourceData);
    glCheckErrors();
    return true;
}

bool GpuTexture2D::Create(ePixelFormat textureFormat, int numMipmaps, const Texture2DMip* mipmaps)
{
    cxx_assert(mipmaps && (numMipmaps > 0));

    GLuint formatGL = GetTextureInputFormatGL(textureFormat);
    GLint internalFormatGL = GetTextureInternalFormatGL(textureFormat);
    GLenum dataType = GetTextureDataTypeGL(textureFormat);
    if (formatGL == 0 || internalFormatGL == 0 || dataType == 0)
    {
        cxx_assert(false);
        return false;
    }

    mPixelFormat = textureFormat;
    mDimensions.x = mipmaps[0].mSizex;
    mDimensions.y = mipmaps[0].mSizey;
    mHasMipmaps = (numMipmaps > 1);
    
    ScopedBinder scopedBinder(this);

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
    glCheckErrors();

    for (int imipmap = 0; imipmap < numMipmaps; ++imipmap)
    {
        cxx_assert(mipmaps[imipmap].mSizex > 0);
        cxx_assert(mipmaps[imipmap].mSizey > 0);
        ::glTexImage2D(GL_TEXTURE_2D, imipmap, internalFormatGL, 
            mipmaps[imipmap].mSizex, 
            mipmaps[imipmap].mSizey, 
            0, 
            formatGL, dataType, mipmaps[imipmap].mPixelsData);
        glCheckErrors();
    }
    return true;
}

void GpuTexture2D::SetSamplerState(eTextureFiltering filtering, eTextureRepeating repeating)
{
    ScopedBinder scopedBinder(this);

    mFiltering = filtering;
    mRepeating = repeating;

    // set filtering
    GLint magFilterGL = GL_NEAREST;
    GLint minFilterGL = GL_NEAREST;
    switch (filtering)
    {
        case eTextureFiltering_None: 
        break;
        case eTextureFiltering_Bilinear:
            if (mHasMipmaps)
            {
                magFilterGL = GL_LINEAR;
                minFilterGL = GL_LINEAR_MIPMAP_NEAREST;
            }
            else
            {
                magFilterGL = GL_LINEAR;
                minFilterGL = GL_LINEAR;
            }
        break;
        case eTextureFiltering_Trilinear:
            if (mHasMipmaps)
            {
                magFilterGL = GL_LINEAR;
                minFilterGL = GL_LINEAR_MIPMAP_LINEAR;
            }
            else
            {
                magFilterGL = GL_LINEAR;
                minFilterGL = GL_LINEAR;
            }
        break;
        default:
        {
            cxx_assert(filtering == eTextureFiltering_None);
        }
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
        case eTextureRepeating_Repeat:
            wrapSGL = GL_REPEAT;
            wrapTGL = GL_REPEAT;
        break;
        case eTextureRepeating_ClampToEdge:
        break;
        default:
        {
            cxx_assert(repeating == eTextureRepeating_Repeat);
        }
        break;
    }

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapSGL);
    glCheckErrors();

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapTGL);
    glCheckErrors();
}

bool GpuTexture2D::IsTextureBound(eTextureUnit textureUnit) const
{
    cxx_assert(textureUnit < eTextureUnit_COUNT);
    return sCurrentTextures2D[textureUnit] == this;
}

bool GpuTexture2D::Upload(const void* sourceData, int mipLevel)
{
    static const Point2D zeroOffset {0, 0};
    return Upload(zeroOffset, mDimensions, sourceData, mipLevel);
}

bool GpuTexture2D::Upload(const Point2D& offset, const Point2D& dimensions, const void* sourceData, int mipLevel)
{
    cxx_assert(mipLevel >= 0);
    cxx_assert((offset.x >= 0) && (offset.y >= 0));
    cxx_assert((dimensions.x > 0) && (dimensions.y > 0));

    if ((mResourceHandle == 0) || (sourceData == nullptr))
        return false;

    const GLuint formatGL = GetTextureInputFormatGL(mPixelFormat);
    const GLenum dataType = GetTextureDataTypeGL(mPixelFormat);

    ScopedBinder scopedBinder(this);
    ::glTexSubImage2D(GL_TEXTURE_2D, mipLevel, offset.x, offset.y,
        dimensions.x, 
        dimensions.y, 
        formatGL, dataType, sourceData);
    glCheckErrors();
    return true;
}
