#include "pch.h"
#include "GpuTexture2D.h"
#include "OpenGL_Defs.h"
#include "GraphicsDeviceContext.h"
#include "GraphicsDevice.h"
#include "Texture2D_Image.h"

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

GpuTexture2D::GpuTexture2D(GraphicsDeviceContext& graphicsContext)
    : mGraphicsContext(graphicsContext)
    , mResourceHandle(GpuTextureHandle_NULL)
    , mSamplerState()
    , mDesc()
{
}

GpuTexture2D::~GpuTexture2D()
{
    FreeTextureObject();
}

bool GpuTexture2D::InitTextureObject(const Texture2D_Desc& textureDesc, const void* sourceData)
{
    if (IsInitialized())
    {   
        debug_assert(false);
        return false;
    }

    // invalid format
    if (textureDesc.mTextureFormat == eTextureFormat_Null)
    {
        debug_assert(false);
        return false;
    }

    // expect pot dimensions
    bool is_pot_x = cxx::get_next_pot(textureDesc.mDimensions.x) == textureDesc.mDimensions.x;
    bool is_pot_y = cxx::get_next_pot(textureDesc.mDimensions.y) == textureDesc.mDimensions.y;

    if (!is_pot_x || !is_pot_y)
    {
        debug_assert(false);
        return false;
    }

    GLuint gl_format = GetTextureInputFormatGL(textureDesc.mTextureFormat);
    GLenum gl_data_type = GetTextureDataTypeGL(textureDesc.mTextureFormat);
    GLint gl_format_internal = GetTextureInternalFormatGL(textureDesc.mTextureFormat);
    if (gl_format == 0 || gl_format_internal == 0 || gl_data_type == 0)
    {
        debug_assert(false);
        return false;
    }

    ::glGenTextures(1, &mResourceHandle);
    glCheckError();

    if (mResourceHandle == GpuTextureHandle_NULL)
    {
        debug_assert(false);
        return false;
    }

    // init texture data
    mDesc = textureDesc;

    ScopeBinder scopedBind {this};

    ::glTexImage2D(GL_TEXTURE_2D, 0, gl_format_internal, mDesc.mDimensions.x, mDesc.mDimensions.y, 0, gl_format, gl_data_type, sourceData);
    glCheckError();

    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mDesc.mMipmapsCount);
    glCheckError();

    // init mipmaps
    if (mDesc.mMipmapsCount > 0)
    {
        if (!mDesc.mGenerateMipmaps)
        {
            for (int iMipmap = 1; iMipmap < mDesc.mMipmapsCount + 1; ++iMipmap)
            {
                int mipmapDimsx = GetTextureMipmapDims(mDesc.mDimensions.x, iMipmap);
                int mipmapDimsy = GetTextureMipmapDims(mDesc.mDimensions.y, iMipmap);

                ::glTexImage2D(GL_TEXTURE_2D, iMipmap, gl_format_internal, mipmapDimsx, mipmapDimsy, 0, gl_format, gl_data_type, nullptr);
                glCheckError();
            }
        }
        else
        {
            GenerateMipmapsImpl();
        }
    }

    // set default filter and repeat mode for texture
    SetSamplerStateImpl();
    return true;
}

void GpuTexture2D::FreeTextureObject()
{
    if (IsInitialized())
    {
        SetUnbound();

        // destroy opengl object
        ::glDeleteTextures(1, &mResourceHandle);
        glCheckError();
    }
    // clear desc
    mDesc = Texture2D_Desc();
}

bool GpuTexture2D::SetSamplerState(const TextureSamplerState& samplerState)
{
    if (!IsInitialized())
    {
        debug_assert(false);
        return false;
    }

    if (mSamplerState == samplerState)
        return true;

    mSamplerState = samplerState;

    ScopeBinder scopedBind {this};
    SetSamplerStateImpl();
    return true;
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

bool GpuTexture2D::IsInitialized() const
{
    return mResourceHandle != GpuTextureHandle_NULL;
}

bool GpuTexture2D::TexSubImage(int mipmapLevel, const Rectangle& rc, const void* sourceData)
{
    if (!IsInitialized())
    {
        debug_assert(false);
        return false;
    }

    debug_assert(mipmapLevel <= mDesc.mMipmapsCount);

    if (sourceData == nullptr)
        return true;

    int mipmapDimX = GetTextureMipmapDims(mDesc.mDimensions.x, mipmapLevel);
    int mipmapDimY = GetTextureMipmapDims(mDesc.mDimensions.y, mipmapLevel);

    debug_assert(rc.x >= 0);
    debug_assert(rc.y >= 0);
    debug_assert(rc.x + rc.w <= mipmapDimX);
    debug_assert(rc.y + rc.h <= mipmapDimY);

    GLuint gl_format = GetTextureInputFormatGL(mDesc.mTextureFormat);
    GLenum gl_data_type = GetTextureDataTypeGL(mDesc.mTextureFormat);
    if (gl_format == 0 || gl_data_type == 0)
    {
        debug_assert(false);
        return false;
    }

    ScopeBinder scopedBind {this};
    ::glTexSubImage2D(GL_TEXTURE_2D, mipmapLevel, rc.x, rc.y, rc.w, rc.h, gl_format, gl_data_type, sourceData);
    glCheckError();
    return true;
}

bool GpuTexture2D::TexSubImage(int mipmapLevel, const Point& dimensions, const void* sourceData)
{
    if (!IsInitialized())
    {
        debug_assert(false);
        return false;
    }

    if (sourceData)
    {
        Rectangle textureRect 
        {
            0,
            0,
            dimensions.x,
            dimensions.y
        };
        return TexSubImage(mipmapLevel, textureRect, sourceData);
    }
    return true;
}

bool GpuTexture2D::TexSubImage(int mipmapLevel, const void* sourceData)
{
    if (!IsInitialized())
    {
        debug_assert(false);
        return false;
    }

    if (sourceData)
    {
        Rectangle textureRect 
        {
            0,
            0,
            GetTextureMipmapDims(mDesc.mDimensions.x, mipmapLevel),
            GetTextureMipmapDims(mDesc.mDimensions.y, mipmapLevel),
        };
        return TexSubImage(mipmapLevel, textureRect, sourceData);
    }
    return true;
}

void GpuTexture2D::SetSamplerStateImpl()
{
    // set filtering
    GLint magFilterGL = GL_NEAREST;
    GLint minFilterGL = mDesc.mMipmapsCount > 0 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    switch (mSamplerState.mFilterMode)
    {
        case eTextureFilterMode_Nearest: 
            break;
        case eTextureFilterMode_Bilinear:
            magFilterGL = GL_LINEAR;
            minFilterGL = mDesc.mMipmapsCount > 0 ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
            break;
        case eTextureFilterMode_Trilinear:
            magFilterGL = GL_LINEAR;
            minFilterGL = mDesc.mMipmapsCount > 0 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
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

void GpuTexture2D::GenerateMipmapsImpl()
{
    ::glGenerateMipmap(GL_TEXTURE_2D);
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
