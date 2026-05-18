#pragma once

#include "GraphicsDefs.h"
#include "Texture.h"
#include "ShaderProgram.h"
    
// Defines rendering properties of surface
class SurfaceMaterial
{
public:
    SurfaceMaterial() = default;
 
    // activate this meterial, render program should be bound to current render device
    void BindMaterial(ShaderProgram& shaderProgram) const;
    void Clear();

    inline bool operator < (const SurfaceMaterial& other) const
    {
        if (mDiffuseTexture != other.mDiffuseTexture)
        {
            return mDiffuseTexture < other.mDiffuseTexture;
        }

        if (mEnvMappingTexture != other.mEnvMappingTexture)
        {
            return mEnvMappingTexture < other.mEnvMappingTexture;
        }

        return mRenderStates < other.mRenderStates;
    }

    // Test whether material is fully opaque
    inline bool IsOpaque() const { return !mRenderStates.mIsAlphaBlendEnabled; }

public:
    Texture* mDiffuseTexture = nullptr;
    Texture* mEnvMappingTexture = nullptr;
    RenderStates mRenderStates;

    // colors
    Color32 mBaseColor = COLOR_WHITE; // for modulation
    //Color32 mSpecularColor = COLOR_WHITE;
    //Color32 mAmbientColor = COLOR_WHITE;
    Color32 mEmissiveColor = COLOR_BLACK;
    
    float mOpacity = 1.0f;
};