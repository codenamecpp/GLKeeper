#pragma once

#include "GraphicsDefs.h"

// forwards
class Texture2D;

// various render properties of drawable object
class RenderMaterial
{
public:
    RenderMaterial();

    // activate or deactivate render material
    void ActivateMaterial();
    void DeactivateMaterial();

    void PreloadTextures();

    void Clear();
    void SetTransparent(bool isTransparent);

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

public:
    std::string mInternalName;

    Texture2D* mDiffuseTexture = nullptr;
    Texture2D* mEnvMappingTexture = nullptr;

    RenderStates mRenderStates;    
};