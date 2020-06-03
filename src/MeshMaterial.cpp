#include "pch.h"
#include "MeshMaterial.h"
#include "GraphicsDevice.h"
#include "Texture2D.h"
#include "RenderManager.h"

MeshMaterial::MeshMaterial() : mMaterialColor(Color32_White)
{
}

void MeshMaterial::Clear()
{
    mRenderStates = RenderStates();

    mDiffuseTexture = nullptr;
    mEnvMappingTexture = nullptr;
    mMaterialColor = Color32_White;
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

    gRenderManager.HandleMaterialActivate(this);
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
    return (mRenderStates == other.mRenderStates) && (mDiffuseTexture == other.mDiffuseTexture) && 
        (mEnvMappingTexture == other.mEnvMappingTexture) && (mMaterialColor == other.mMaterialColor);
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

    if (mMaterialColor != other.mMaterialColor)
        return mMaterialColor < other.mMaterialColor;

    return false;
}