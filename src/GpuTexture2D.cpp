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

bool GpuTexture2D::Create(ePixelFormat textureFormat, int sizex, int sizey, const void* sourceData)
{
    GLuint formatGL = GetTextureInputFormatGL(textureFormat);
    GLint internalFormatGL = GetTextureInternalFormatGL(textureFormat);
    GLenum dataType = GetTextureDataTypeGL(textureFormat);
    if (formatGL == 0 || internalFormatGL == 0 || dataType == 0)
    {
        cxx_assert(false);
        return false;
    }

    mPixelFormat = textureFormat;
    mDimensions.x = sizex;
    mDimensions.y = sizey;
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

    if (numMipmaps > 1)
    {
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numMipmaps - 1);
        glCheckErrors();
    }

    for (int imipmap = 0; imipmap < numMipmaps; ++imipmap)
    {
        cxx_assert(mipmaps[imipmap].mSizex > 0);
        cxx_assert(mipmaps[imipmap].mSizey > 0);
        if (mipmaps[imipmap].mPixelsData == nullptr)
            continue;

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

void GpuTexture2D::Invalidate()
{
    if (mResourceHandle == 0)
        return;

    GLuint formatGL = GetTextureInputFormatGL(mPixelFormat);
    GLint internalFormatGL = GetTextureInternalFormatGL(mPixelFormat);
    GLenum dataType = GetTextureDataTypeGL(mPixelFormat);

    ScopedBinder scopedBinder(this);
    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, 
        mDimensions.x, 
        mDimensions.y, 
        0, 
        formatGL, dataType, nullptr);
    glCheckErrors();
}

bool GpuTexture2D::Upload(const void* sourceData)
{
    if (mResourceHandle == 0 || sourceData == nullptr)
        return false;

    GLuint formatGL = GetTextureInputFormatGL(mPixelFormat);
    GLint internalFormatGL = GetTextureInternalFormatGL(mPixelFormat);
    GLenum dataType = GetTextureDataTypeGL(mPixelFormat);

    ScopedBinder scopedBinder(this);
    ::glTexImage2D(GL_TEXTURE_2D, 0, internalFormatGL, 
        mDimensions.x, 
        mDimensions.y, 
        0, 
        formatGL, dataType, sourceData);
    glCheckErrors();
    return true;
}
