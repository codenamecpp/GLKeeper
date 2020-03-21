#pragma once

#include "GraphicsDefs.h"
#include "ResourceDefs.h"

// various render properties of drawable object
class RenderMaterial
{
public:
    RenderMaterial();

    // activate or deactivate render material
    void ActivateMaterial();

    void PreloadTextures();
    void Clear();

    // test whether material must be drawn with transparency
    inline bool IsTransparent() const
    {
        return mRenderStates.mIsAlphaBlendEnabled;
    }

    // operators
    inline bool operator == (const RenderMaterial& other) const
    {
        return mRenderStates == other.mRenderStates && mDiffuseTexture == other.mDiffuseTexture && 
            mEnvMappingTexture == other.mEnvMappingTexture; 
    }

    inline bool operator != (const RenderMaterial& other) const
    {
        return mRenderStates != other.mRenderStates || mDiffuseTexture != other.mDiffuseTexture || 
            mEnvMappingTexture != other.mEnvMappingTexture; 
    }

    inline bool operator < (const RenderMaterial& other) const
    {
        if (mDiffuseTexture != other.mDiffuseTexture)
            return mDiffuseTexture < other.mDiffuseTexture;

        if (mEnvMappingTexture != other.mEnvMappingTexture)
            return mEnvMappingTexture < other.mEnvMappingTexture;

        if (mRenderStates != other.mRenderStates)
            return mRenderStates < other.mRenderStates;

        return false;
    }

public:
    Texture2D* mDiffuseTexture = nullptr;
    Texture2D* mEnvMappingTexture = nullptr;

    RenderStates mRenderStates;    
};