#include "stdafx.h"
#include "SurfaceMaterial.h"
#include "RenderDevice.h"
#include "TextureManager.h"

void SurfaceMaterial::BindMaterial(ShaderProgram& shaderProgram) const
{
    gRenderDevice.SetRenderState(mRenderStates);
    if (mDiffuseTexture)
    {
        mDiffuseTexture->BindTexture(eTextureUnit_DiffuseMap0);
    }
    shaderProgram.SetMaterialUniforms(*this);
}

void SurfaceMaterial::Clear()
{
    mDiffuseTexture = {};
    mEnvMappingTexture = {};

    mBaseColor = COLOR_WHITE;
    //mSpecularColor = COLOR_WHITE;
    //mAmbientColor = COLOR_WHITE;
    mEmissiveColor = COLOR_BLACK;

    mOpacity = 1.0f;
}
