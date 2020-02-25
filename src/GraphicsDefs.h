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
class Texture2D_Data;

// internal types
using GpuProgramHandle = unsigned int;
using GpuBufferHandle = unsigned int;
using GpuTextureHandle = unsigned int;
using GpuVertexArrayHandle = unsigned int;
using GpuVariableLocation = int;

// predefined value for unspecified render program variable location
const GpuVariableLocation GpuVariable_NULL = -1;

//////////////////////////////////////////////////////////////////////////

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

enum eTextureWrapMode
{
    eTextureWrapMode_Repeat, 
    eTextureWrapMode_ClampToEdge,
    eTextureWrapMode_COUNT
};

decl_enum_strings(eTextureWrapMode);

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

inline int GetTextureMipmapDimensions(int dimensions, int mipmap)
{
    if (mipmap < 1)
        return dimensions;

    if (dimensions < 1)
        return 0;

    dimensions = dimensions >> mipmap;
    if (dimensions < 1)
    {
        dimensions = 1;
    }
    return dimensions;
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
struct RenderStates
{
public:
    RenderStates()
        : mIsColorWriteEnabled(true)
        , mIsDepthWriteEnabled(true)
        , mIsDepthTestEnabled(true)
        , mIsFaceCullingEnabled(true)
        , mIsAlphaBlendEnabled() // is disabled by default, do not draw dungeon geometry with it
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
        states.mBlendingMode = eBlendingMode_Alpha;
        return states;
    }

public:
    // polygon fill mode state
    ePolygonFillMode mPolygonFillMode = ePolygonFillMode_Solid;

    // blend mode state
    eBlendingMode mBlendingMode = eBlendingMode_Alpha;

    // cull mode state
    eCullingMode mCullingMode = eCullingMode_Back;

    // depth func state
    eDepthTestFunc mDepthFunc = eDepthTestFunc_LessEqual;

    // state flags
    bool mIsAlphaBlendEnabled : 1;
    bool mIsColorWriteEnabled : 1;
    bool mIsDepthWriteEnabled : 1;
    bool mIsDepthTestEnabled : 1;
    bool mIsFaceCullingEnabled : 1;
};

const unsigned int Sizeof_RenderStates = sizeof(RenderStates);

inline bool operator == (const RenderStates& a, const RenderStates& b) { return ::memcmp(&a, &b, Sizeof_RenderStates) == 0; }
inline bool operator != (const RenderStates& a, const RenderStates& b) { return ::memcmp(&a, &b, Sizeof_RenderStates) != 0; }

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

