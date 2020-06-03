#pragma once

enum eMaterialColorMode
{
    eMaterialColorMode_Texture,     // diffuse texture * vertex color
    eMaterialColorMode_TextureAdd,  // diffuse texture + vertex color
    eMaterialColorMode_Vertex,      // vertex color
};

// various render properties of drawable object
class MeshMaterial
{
public:
    MeshMaterial();

    // activate or deactivate render material
    void ActivateMaterial();

    void PreloadTextures();
    void Clear();

    // test whether material must be drawn with transparency
    inline bool IsTransparent() const
    {
        return mRenderStates.mIsAlphaBlendEnabled;
    }

    // comparsion operators
    bool operator == (const MeshMaterial& other) const;
    bool operator != (const MeshMaterial& other) const;
    bool operator < (const MeshMaterial& other) const;

public:
    Texture2D* mDiffuseTexture = nullptr;
    Texture2D* mEnvMappingTexture = nullptr;
    RenderStates mRenderStates;
    eMaterialColorMode mColorMode = eMaterialColorMode_Texture;
};