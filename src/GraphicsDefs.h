#pragma once

#include "CommonTypes.h"

// forwards
class GraphicsDevice;
class GpuBuffer;
class GpuProgram;
class GpuBufferTexture;
class GpuTexture2D;
class GpuTextureArray2D;
class GraphicsDeviceContext;
class Texture2D_Image;

// internal types
using GpuProgramHandle = unsigned int;
using GpuBufferHandle = unsigned int;
using GpuTextureHandle = unsigned int;
using GpuVertexArrayHandle = unsigned int;
using GpuVariableLocation = int;

// predefined values
const GpuVariableLocation GpuVariable_NULL = -1;
const GpuTextureHandle GpuTextureHandle_NULL = 0;
const GpuProgramHandle GpuProgramHandle_NULL = 0;

// render vertex 3d
struct Vertex3D
{
public:
    glm::vec3 mPosition; // 12 bytes
    glm::vec3 mNormal; // 12 bytes
    glm::vec2 mTexcoord; // 8 bytes
    Color32 mColor; // 4 bytes
};

const unsigned int Sizeof_Vertex3D = sizeof(Vertex3D);

// render vertex 2d
struct Vertex2D
{
public:
    glm::vec2 mPosition; // 8 bytes
    glm::vec2 mTexcoord; // 8 bytes
    Color32 mColor; // 4 bytes
};

const unsigned int Sizeof_Vertex2D = sizeof(Vertex2D);

// render vertex 3d debug
struct Vertex3D_Debug
{
public:
    glm::vec3 mPosition; // 12 bytes
    Color32 mColor; // 4 bytes
};

const unsigned int Sizeof_Vertex3D_Debug = sizeof(Vertex3D_Debug);

// render vertex 3d terrain mesh
struct Vertex3D_Terrain
{
public:
    glm::vec3 mPosition; // 12 bytes
    glm::vec3 mNormal; // 12 bytes
    glm::vec2 mTexcoord; // 8 bytes
                         // terrain tile logical coordinate
    union // 4 bytes
    {
        unsigned int mTileCoord;
        struct
        {
            unsigned short mTileX;
            unsigned short mTileY;
        };
    };
};

const unsigned int Sizeof_Vertex3D_Terrain = sizeof(Vertex3D_Terrain);

//////////////////////////////////////////////////////////////////////////

enum eTextureFilterMode
{
    eTextureFilterMode_Nearest,
    eTextureFilterMode_Bilinear, 
    eTextureFilterMode_Trilinear,
    eTextureFilterMode_COUNT
};

decl_enum_strings(eTextureFilterMode);

enum eTextureRepeatMode
{
    eTextureRepeatMode_Repeat, 
    eTextureRepeatMode_ClampToEdge,
    eTextureRepeatMode_COUNT
};

decl_enum_strings(eTextureRepeatMode);

enum eTextureFormat
{
    eTextureFormat_Null,
    eTextureFormat_R8,
    eTextureFormat_R8_G8,
    eTextureFormat_RGB8,
    eTextureFormat_RGBA8,
    eTextureFormat_RGBA8UI,
    eTextureFormat_R8UI,
    eTextureFormat_R16UI,
    eTextureFormat_COUNT
};

decl_enum_strings(eTextureFormat);

// texture sampler state
struct TextureSamplerState
{
public:
    TextureSamplerState() = default;
    TextureSamplerState(eTextureFilterMode filter)
        : mFilterMode(filter)
    {
    }
    TextureSamplerState(eTextureFilterMode filter, eTextureRepeatMode repeat)
        : mFilterMode(filter)
        , mRepeatModeS(repeat)
        , mRepeatModeT(repeat)
        , mRepeatModeR(repeat)
    {
    }
public:
    eTextureFilterMode mFilterMode = eTextureFilterMode_Nearest;
    eTextureRepeatMode mRepeatModeS = eTextureRepeatMode_ClampToEdge; // wrap x coord
    eTextureRepeatMode mRepeatModeT = eTextureRepeatMode_ClampToEdge; // wrap y coord
    eTextureRepeatMode mRepeatModeR = eTextureRepeatMode_ClampToEdge; // wrap z coord
};

// compare operators
inline bool operator == (const TextureSamplerState& lhs, const TextureSamplerState& rhs)
{
    return lhs.mFilterMode == rhs.mFilterMode &&
        lhs.mRepeatModeS == rhs.mRepeatModeS &&
        lhs.mRepeatModeT == rhs.mRepeatModeT &&
        lhs.mRepeatModeR == rhs.mRepeatModeR;
}

inline bool operator != (const TextureSamplerState& lhs, const TextureSamplerState& rhs)
{
    return !(lhs == rhs);
}

// texture2d description structure
struct Texture2D_Desc
{
public:
    Texture2D_Desc(): mDimensions(), mImageDimensions()
        , mMaxU(1.0f)
        , mMaxV(1.0f)
        , mMipmapsCount() // default no mipmaps
        , mTransparent() // default is opaque
        , mGenerateMipmaps() // do not generate on upload
    {
    }
    Texture2D_Desc(eTextureFormat format, const Size2D& dimensions, int mipmapsCount, bool transparent, bool generateMipmaps)
        : mDimensions(dimensions), mImageDimensions(dimensions)
        , mTextureFormat(format)
        , mMaxU(1.0f)
        , mMaxV(1.0f)
        , mMipmapsCount(mipmapsCount)
        , mTransparent(transparent)
        , mGenerateMipmaps(generateMipmaps)
    {
    }

    // texture format of base level and of all mipmaps is the same
    eTextureFormat mTextureFormat = eTextureFormat_Null;

    // base level texture dimensions
    Size2D mDimensions;
    Size2D mImageDimensions; // NPOT size of texture image

    // normalized image size uv
    float mMaxU;
    float mMaxV;

    int mMipmapsCount; // 0 means that mipmaps are disabled
    // texture has transparent pixels
    bool mTransparent; 
    bool mGenerateMipmaps; // auto generate mipmaps on data upload
};

// get dimensions of specific texture mipmap level
// @param baseDimensions: Base texture dimensions
// @param mipmapLevel: Mipmap, 0 is base texture dimensions
inline int GetTextureMipmapDims(int baseDimensions, int mipmapLevel)
{
    if (mipmapLevel > 0)
    {
        baseDimensions = (baseDimensions >> mipmapLevel);
    }

    if (baseDimensions < 1)
    {
        baseDimensions = 1;
    }
    return baseDimensions;
}

// get number of bytes per pixel for specific texture format
// @param format: Format identifier
inline int GetTextureFormatBytesCount(eTextureFormat format) 
{
    debug_assert(format < eTextureFormat_COUNT && format > eTextureFormat_Null);
    switch (format)
    {
        case eTextureFormat_R8UI: return 1;
        case eTextureFormat_R8: return 1;
        case eTextureFormat_R16UI: return 2;
        case eTextureFormat_R8_G8: return 2;
        case eTextureFormat_RGB8: return 3;
        case eTextureFormat_RGBA8: return 4;
        case eTextureFormat_RGBA8UI: return 4;
    }
    return 0;
}

// get number of bits per pixel for specific pixel format
// @param format: Format identifier
inline int GetTextureFormatBitsCount(eTextureFormat format) 
{
    int numBytes = GetTextureFormatBytesCount(format);
    return numBytes * 8;
}

// compute texture data size in bytes
// @param format: Texture format
// @param baseImageDims: Base image dimensions
// @param mipmapLevel: Texture mipmap, 0 is for base image
inline int GetTextureDataSize(eTextureFormat format, const Size2D& baseImageDims, int mipmapLevel)
{
    int dimx = GetTextureMipmapDims(baseImageDims.x, mipmapLevel);
    int dimy = GetTextureMipmapDims(baseImageDims.y, mipmapLevel);
    return dimx * dimy * GetTextureFormatBytesCount(format);
}

enum ePrimitiveType
{
    ePrimitiveType_Points, 
    ePrimitiveType_Lines, 
    ePrimitiveType_LineLoop, 
    ePrimitiveType_Triangles,
    ePrimitiveType_TriangleStrip,
    ePrimitiveType_TriangleFan,
    ePrimitiveType_COUNT
};

decl_enum_strings(ePrimitiveType);

enum eIndicesType
{
    eIndicesType_i16, 
    eIndicesType_i32, 
    eIndicesType_COUNT
};

decl_enum_strings(eIndicesType);

enum eTextureUnit
{
    eTextureUnit_0,
    eTextureUnit_1,
    eTextureUnit_2,
    eTextureUnit_3,
    eTextureUnit_4,
    eTextureUnit_5,
    eTextureUnit_6,
    eTextureUnit_7,
    eTextureUnit_8,
    eTextureUnit_9,
    eTextureUnit_10,
    eTextureUnit_11,
    eTextureUnit_12,
    eTextureUnit_13,
    eTextureUnit_14,
    eTextureUnit_15,
    eTextureUnit_COUNT = 16, // not valid texture unit
};

decl_enum_strings(eTextureUnit);

//////////////////////////////////////////////////////////////////////////

// standard vertex attributes
enum eVertexAttributeFormat
{
    eVertexAttributeFormat_2F,      // 2 floats
    eVertexAttributeFormat_3F,      // 3 floats
    eVertexAttributeFormat_4UB,     // 4 unsigned bytes
    eVertexAttributeFormat_1US,     // 1 unsigned short
    eVertexAttributeFormat_2US,     // 2 unsigned shorts
    eVertexAttributeFormat_Unknown
};

decl_enum_strings(eVertexAttributeFormat);

enum eVertexAttribute
{
    eVertexAttribute_Position0,
    eVertexAttribute_Position1,
    eVertexAttribute_Texcoord0,
    eVertexAttribute_Texcoord1,
    eVertexAttribute_Normal0,
    eVertexAttribute_Normal1,
    eVertexAttribute_Color0,
    eVertexAttribute_Color1,
    eVertexAttribute_TerrainTilePosition,
    eVertexAttribute_COUNT,
    eVertexAttribute_MAX = 16,
};

decl_enum_strings(eVertexAttribute);

// get number of component for vertex attribute
// @param attributeSemantics: Attribute semantics
inline unsigned int GetAttributeComponentCount(eVertexAttributeFormat attributeFormat)
{
    switch (attributeFormat)
    {
        case eVertexAttributeFormat_2F: return 2;
        case eVertexAttributeFormat_3F: return 3;
        case eVertexAttributeFormat_4UB: return 4;
        case eVertexAttributeFormat_1US: return 1;
        case eVertexAttributeFormat_2US: return 2;
    }
    debug_assert(false);
    return 0;
}

// get vertex attribute size in bytes
// @param attributeSemantics: Attribute semantics
inline unsigned int GetAttributeSizeBytes(eVertexAttributeFormat attributeFormat)
{
    switch (attributeFormat)
    {
        case eVertexAttributeFormat_2F: return 2 * sizeof(float);
        case eVertexAttributeFormat_3F: return 3 * sizeof(float);
        case eVertexAttributeFormat_4UB: return 4 * sizeof(unsigned char);
        case eVertexAttributeFormat_1US: return 1 * sizeof(unsigned short);
        case eVertexAttributeFormat_2US: return 2 * sizeof(unsigned short);
    }
    debug_assert(false);
    return 0;
}

//////////////////////////////////////////////////////////////////////////

enum eBufferContent
{
    eBufferContent_Vertices,
    eBufferContent_Indices,
    eBufferContent_COUNT
};

decl_enum_strings(eBufferContent);

enum eBufferUsage
{
    eBufferUsage_Static, // The data store contents will be modified once and used many times
    eBufferUsage_Dynamic, // The data store contents will be modified repeatedly and used many times
    eBufferUsage_Stream, // The data store contents will be modified once and used at most a few times
    eBufferUsage_COUNT
};

decl_enum_strings(eBufferUsage);

enum BufferAccessBits : unsigned short
{
    BufferAccess_Read  = (1 << 0),
    BufferAccess_Write = (1 << 1),
    BufferAccess_Unsynchronized = (1 << 2), // client must to guarantee that mapped buffer region is doesn't used by the GPU
    BufferAccess_InvalidateRange = (1 << 3), // have meaning only for range lock
    BufferAccess_InvalidateBuffer = (1 << 4), // orphan whole buffer
};

inline BufferAccessBits operator | (BufferAccessBits lhs, BufferAccessBits rhs)
{
    return static_cast<BufferAccessBits>(static_cast<unsigned short>(lhs) | static_cast<unsigned short>(rhs));
}
inline BufferAccessBits operator & (BufferAccessBits lhs, BufferAccessBits rhs)
{
    return static_cast<BufferAccessBits>(static_cast<unsigned short>(lhs) & static_cast<unsigned short>(rhs));
}
inline BufferAccessBits operator ^ (BufferAccessBits lhs, BufferAccessBits rhs)
{
    return static_cast<BufferAccessBits>(static_cast<unsigned short>(lhs) ^ static_cast<unsigned short>(rhs));
}

const BufferAccessBits BufferAccess_UnsynchronizedWrite = BufferAccess_Unsynchronized | BufferAccess_Write;

//////////////////////////////////////////////////////////////////////////

enum eBlendingMode : unsigned char
{
    eBlendingMode_Alpha,
    eBlendingMode_Additive,
    eBlendingMode_Multiply,
    eBlendingMode_Premultiplied,
    eBlendingMode_Screen
};

decl_enum_strings(eBlendingMode);

enum eDepthTestFunc : unsigned char
{
    eDepthTestFunc_Always,
    eDepthTestFunc_Equal,
    eDepthTestFunc_NotEqual,
    eDepthTestFunc_Less,
    eDepthTestFunc_Greater,
    eDepthTestFunc_LessEqual,
    eDepthTestFunc_GreaterEqual
};

decl_enum_strings(eDepthTestFunc);

enum eCullingMode : unsigned char
{
    eCullingMode_Front,
    eCullingMode_Back,
    eCullingMode_FrontAndBack,
};

decl_enum_strings(eCullingMode);

enum ePolygonFillMode : unsigned char
{
    ePolygonFillMode_Solid,
    ePolygonFillMode_WireFrame,
};

decl_enum_strings(ePolygonFillMode);

// defines render states
union RenderStates
{
public:
    RenderStates()
        : mIsColorWriteEnabled(true)
        , mIsDepthWriteEnabled(true)
        , mIsDepthTestEnabled(true)
        , mIsFaceCullingEnabled(true)
        , mIsAlphaBlendEnabled() // is disabled by default, do not draw dungeon geometry with it
        , mPolygonFillMode(ePolygonFillMode_Solid)
        , mBlendingMode(eBlendingMode_Alpha)
        , mCullingMode(eCullingMode_Back)
        , mDepthFunc(eDepthTestFunc_LessEqual)
    {
    }
    // get default render states for ui drawing
    static RenderStates GetForUI()
    {
        RenderStates states;
        states.mIsDepthWriteEnabled = false;
        states.mIsDepthTestEnabled = false;
        states.mIsFaceCullingEnabled = false;
        states.mIsAlphaBlendEnabled = true;
        return states;
    }
    // comparsion operators
    inline bool operator == (const RenderStates& renderStates) const { return mSortKey == renderStates.mSortKey; }
    inline bool operator != (const RenderStates& renderStates) const { return mSortKey != renderStates.mSortKey; }
    inline bool operator < (const RenderStates& renderStates) const { return mSortKey < renderStates.mSortKey; }

public:

    struct
    {
        // render state parameters
        ePolygonFillMode mPolygonFillMode;
        eBlendingMode mBlendingMode; 
        eCullingMode mCullingMode;
        eDepthTestFunc mDepthFunc;

        // render state flags
        bool mIsAlphaBlendEnabled : 1;
        bool mIsColorWriteEnabled : 1;
        bool mIsDepthWriteEnabled : 1;
        bool mIsDepthTestEnabled : 1;
        bool mIsFaceCullingEnabled : 1;
    };

    std::uint64_t mSortKey;
};

const unsigned int Sizeof_RenderStates = sizeof(RenderStates);

//////////////////////////////////////////////////////////////////////////

enum eGraphicsDeviceFeature
{
    eGraphicsDeviceFeature_NPOT_Textures,
    eGraphicsDeviceFeature_ABGR,
    eGraphicsDeviceFeature_COUNT
};

struct GraphicsDeviceCaps
{
public:
    int mMaxArrayTextureLayers = 0;
    int mMaxTextureBufferSize = 0;
    bool mFeatures[eGraphicsDeviceFeature_COUNT];
};

//////////////////////////////////////////////////////////////////////////

struct RenderProgramInputLayout
{
public:
    RenderProgramInputLayout() = default;
    // enable vertex attribute
    // @param attributeStream: Attribute stream
    inline void IncludeAttribute(eVertexAttribute attributeStream) 
    {
        if (attributeStream < eVertexAttribute_COUNT) mEnabledAttributes |= (1 << attributeStream);
    }
    // disable vertex attribute
    // @param attributeStream: Attribute stream
    inline void ExcludeAttribute(eVertexAttribute attributeStream) 
    {
        if (attributeStream < eVertexAttribute_COUNT) mEnabledAttributes &= ~(1 << attributeStream);
    }
    // test whether vertex attribute enabled
    // @param attributeStream: Attribute stream
    inline bool HasAttribute(eVertexAttribute attributeStream) const 
    {
        return (attributeStream < eVertexAttribute_COUNT) && (mEnabledAttributes & (1 << attributeStream)) > 0;
    }
public:
    unsigned int mEnabledAttributes = 0;
};

inline bool operator == (const RenderProgramInputLayout& a, const RenderProgramInputLayout& b) { return a.mEnabledAttributes == b.mEnabledAttributes; }
inline bool operator != (const RenderProgramInputLayout& a, const RenderProgramInputLayout& b) { return a.mEnabledAttributes != b.mEnabledAttributes; }

