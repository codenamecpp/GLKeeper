#include "pch.h"
#include "RenderMaterial.h"
#include "GraphicsDevice.h"
#include "Texture2D.h"

RenderMaterial::RenderMaterial()
{
}

void RenderMaterial::Clear()
{
    mRenderStates = RenderStates();

    // reset textures
    mDiffuseTexture = nullptr;
    mEnvMappingTexture = nullptr;
}

void RenderMaterial::ActivateMaterial()
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

void RenderMaterial::PreloadTextures()
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
