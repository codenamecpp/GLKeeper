#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GraphicsDefs.h"
#include "AssetDefs.h"

//////////////////////////////////////////////////////////////////////////
    
// Defines rendering properties of surface
class SurfaceMaterial
{
public:
    SurfaceMaterial() = default;
 
    // activate this meterial, render program should be bound to current render device
    void BindMaterial(ShaderProgram& shaderProgram) const;
    void Clear();

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

//////////////////////////////////////////////////////////////////////////