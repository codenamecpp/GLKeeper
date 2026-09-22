#pragma once

//////////////////////////////////////////////////////////////////////////

class MeshAsset;
class Font;
class ShaderProgram;
class ShaderProgram_DebugDraw;
class ShaderProgram_Terrain;
class ShaderProgram_WaterLava;
class ShaderProgram_BlendFrames;
class ShaderProgram_UI;
class ShaderProgram_StaticMesh;
class Texture;
class TextureAtlas;

//////////////////////////////////////////////////////////////////////////

struct TextureRegion
{
public:
    TextureRegion() = default;
    static TextureRegion GetIdent()
    {
        TextureRegion region;
        region.mRect.x = 0;
        region.mRect.y = 0;
        region.mRect.w = 1;
        region.mRect.h = 1;
        region.mUvMin = {0.0f, 0.0f};
        region.mUvMax = {1.0f, 1.0f};
        return region;
    }
    inline void Init(const Rect2D& region, const Point2D& fullTextureDims)
    {   
        mRect = region;

        cxx_assert(fullTextureDims.x > 0);
        cxx_assert(fullTextureDims.y > 0);
        const float invx = 1.0f / fullTextureDims.x;
        const float invy = 1.0f / fullTextureDims.y;

        mUvMin.x = mRect.x * invx;
        mUvMin.y = mRect.y * invy;
        mUvMax.x = (mRect.x + mRect.w) * invx;
        mUvMax.y = (mRect.y + mRect.h) * invy;
    }
    inline Point2D GetDimensions() const { return mRect.GetSize(); }
public:
    Rect2D mRect {};

    glm::vec2 mUvMin {}; // top-left corner of rect
    glm::vec2 mUvMax {}; // bottom-right corner of rect
};

//////////////////////////////////////////////////////////////////////////

struct TextureAtlasEntry
{
    TextureAtlas* mTextureAtlas {};
    TextureRegion mTextureRegion {};
};

//////////////////////////////////////////////////////////////////////////

enum eTextureBacking
{
    eTextureBacking_None,
    eTextureBacking_Atlas,
    eTextureBacking_Default = eTextureBacking_Atlas,
};

//////////////////////////////////////////////////////////////////////////

// TextureSourceId identifies where the texture data actually resides

// Standalone textures get their own unique ID, while textures packed into the same atlas 
// share the same ID

// This means two Texture instances with different pointers and different contents
// will have the same ID if they are backed by the same atlas

// It is useful for geometry batching: if the underlying GpuTexture hasn't been created yet, 
// this ID can be used to form batches without forcing GPU resource creation

using TextureSourceId = unsigned int;

//////////////////////////////////////////////////////////////////////////