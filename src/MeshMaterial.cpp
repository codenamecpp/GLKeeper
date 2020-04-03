#include "pch.h"
#include "MeshMaterial.h"
#include "GraphicsDevice.h"
#include "Texture2D.h"

MeshMaterial::MeshMaterial()
{
}

void MeshMaterial::Clear()
{
    mRenderStates = RenderStates();

    // reset textures
    mDiffuseTexture = nullptr;
    mEnvMappingTexture = nullptr;
}

void MeshMaterial::ActivateMaterial()
{
    gGraphicsDevice.SetRenderStates(mRenderStates);

    if (mDiffuseTexture)
    {
        if (!mDiffuseTexture->ActivateTexture(eTextureUnit_0))
        {
            debug_assert(false);
        }
    }
}

void MeshMaterial::PreloadTextures()
{
    if (mDiffuseTexture)
    {
        mDiffuseTexture->LoadTexture();
    }

    if (mEnvMappingTexture)
    {
        mEnvMappingTexture->LoadTexture();
    }
}

bool MeshMaterial::operator == (const MeshMaterial& other) const
{
    return mRenderStates == other.mRenderStates && 
        mDiffuseTexture == other.mDiffuseTexture && 
        mEnvMappingTexture == other.mEnvMappingTexture; 
}

bool MeshMaterial::operator != (const MeshMaterial& other) const
{
    return !(*this == other);
}

bool MeshMaterial::operator < (const MeshMaterial& other) const
{
    if (mDiffuseTexture != other.mDiffuseTexture)
        return mDiffuseTexture < other.mDiffuseTexture;

    if (mEnvMappingTexture != other.mEnvMappingTexture)
        return mEnvMappingTexture < other.mEnvMappingTexture;

    if (mRenderStates != other.mRenderStates)
        return mRenderStates < other.mRenderStates;

    return false;
}