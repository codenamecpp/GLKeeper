#pragma once

//////////////////////////////////////////////////////////////////////////

// forwards
class RenderDevice;
class GpuBuffer;
class GpuVertexBuffer;
class GpuIndexBuffer;
class GpuTexture;
class GpuProgram;

//////////////////////////////////////////////////////////////////////////

// internals
using GpuResourceHandle = unsigned int;
using GpuVariableLocation = int; // shader attribute / uniform index
const GpuVariableLocation GpuLocation_Null = -1;

//////////////////////////////////////////////////////////////////////////

// Defines information about single 2d-texture mipmap level
struct Texture2DMip
{
public:
    Texture2DMip() = default;
    Texture2DMip(int sizex, int sizey, const void* pixelsData)
        : mSizex(sizex)
        , mSizey(sizey)
        , mPixelsData(pixelsData)
    {
    }
public:
    int mSizex = 0;
    int mSizey = 0;
    const void* mPixelsData = nullptr;
};

//////////////////////////////////////////////////////////////////////////
// Point Light
//////////////////////////////////////////////////////////////////////////

struct LightParams
{
public:
    LightParams()
        : mLightColor(COLOR_WHITE)
        , mLightOffset()
        , mLightRadius(1.0f)
        , mLightIntensity(1.0f)
        , mFlicker()
        , mPulse()
        , mPlayerColoured()
    {}

    // Set light color
    // @param theR, theG, theB, theA: Color components
    inline void SetLightColor(unsigned char theR, unsigned char theG, unsigned char theB, unsigned char theA)
    {
        mLightColor.SetComponents(theR, theG, theB, theA);
    }

    // Set light offset
    // @param theX, theY, theZ: Offset coordinates
    inline void SetLightOffset(float theX, float theY, float theZ)
    {
        mLightOffset[0] = theX;
        mLightOffset[1] = theY;
        mLightOffset[2] = theZ;
    }

public:
    Color32 mLightColor;
    glm::vec3 mLightOffset;
    float mLightRadius;
    float mLightIntensity;
    bool mFlicker;
    bool mPulse;
    bool mPlayerColoured;
};


//////////////////////////////////////////////////////////////////////////

using RenderLayerMask = unsigned int;

static const RenderLayerMask RenderLayer_WorldTerrain = (1 << 0);
static const RenderLayerMask RenderLayer_WorldObjects = (1 << 1);
static const RenderLayerMask RenderLayer_MeshPreview = (1 << 10);

//////////////////////////////////////////////////////////////////////////

enum eRenderPass
{
    eRenderPass_Opaque,
    eRenderPass_Translucent,
    eRenderPass_COUNT
};

//////////////////////////////////////////////////////////////////////////

// Clear mask
enum eDeviceClearMode
{
    eDeviceClear_ColorDepthBuffers, // both color buffer and depth buffer
    eDeviceClear_ColorBuffer, // color buffer only
    eDeviceClear_DepthBuffer, // depth buffer only
};

//////////////////////////////////////////////////////////////////////////

enum eTextureFiltering
{
    eTextureFiltering_None, 
    eTextureFiltering_Bilinear, 
    eTextureFiltering_Trilinear
};

enum eTextureRepeating
{
    eTextureRepeating_Repeat, 
    eTextureRepeating_ClampToEdge
};

enum ePixelFormat
{
    ePixelFormat_Null,

    ePixelFormat_R8,
    ePixelFormat_R8_G8,
    ePixelFormat_RGB8,
    ePixelFormat_RGBA8,
    ePixelFormat_RGBA8UI,
    ePixelFormat_R8UI,
    ePixelFormat_R16UI,
};

enum_serialize_decl(ePixelFormat);

// Get number of bytes per pixel for specific texture format
// @param format: Format identifier
inline int GetBytesPerPixel(ePixelFormat format) 
{
    switch (format)
    {
        case ePixelFormat_R8:
        case ePixelFormat_R8UI: 
            return 1;
        case ePixelFormat_R8_G8: 
        case ePixelFormat_R16UI:
            return 2;
        case ePixelFormat_RGB8: 
            return 3;
        case ePixelFormat_RGBA8UI:
        case ePixelFormat_RGBA8: 
            return 4;
    }
    cxx_assert(false);
    return 0;
}

//////////////////////////////////////////////////////////////////////////

enum ePrimitiveType
{
    ePrimitiveType_Points, 
    ePrimitiveType_Lines, 
    ePrimitiveType_LineLoop, 
    ePrimitiveType_Triangles,
    ePrimitiveType_TriangleStrip,
    ePrimitiveType_TriangleFan,
};

enum eIndicesType
{
    eIndicesType_i16, 
    eIndicesType_i32, 
    eIndicesType_COUNT
};

// sampler
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

    // predefined texture maps
    eTextureUnit_DiffuseMap0 = eTextureUnit_0,
    eTextureUnit_DiffuseMap1 = eTextureUnit_1,
    eTextureUnit_DiffuseMap2 = eTextureUnit_2,
    eTextureUnit_EnvMap0 = eTextureUnit_3,
};

inline const char* ToString(eTextureUnit textureUnit)
{
    switch (textureUnit)
    {
        case eTextureUnit_0: return "tex_0";
        case eTextureUnit_1: return "tex_1";
        case eTextureUnit_2: return "tex_2";
        case eTextureUnit_3: return "tex_3";
        case eTextureUnit_4: return "tex_4";
        case eTextureUnit_5: return "tex_5";
        case eTextureUnit_6: return "tex_6";
        case eTextureUnit_7: return "tex_7";
        case eTextureUnit_8: return "tex_8";
        case eTextureUnit_9: return "tex_9";
        case eTextureUnit_10: return "tex_10";
        case eTextureUnit_11: return "tex_11";
        case eTextureUnit_12: return "tex_12";
        case eTextureUnit_13: return "tex_13";
        case eTextureUnit_14: return "tex_14";
        case eTextureUnit_15: return "tex_15";
    }
    cxx_assert(false);
    return "";
};

// standard vertex attributes

enum eVertexAttributeFormat
{
    eVertexAttributeFormat_Null,

    eVertexAttributeFormat_2F,      // 2 floats
    eVertexAttributeFormat_3F,      // 3 floats
    eVertexAttributeFormat_4F,      // 4 floats
    eVertexAttributeFormat_4UB,     // 4 unsigned bytes
    eVertexAttributeFormat_1US,     // 1 unsigned short
    eVertexAttributeFormat_2US,     // 2 unsigned shorts
    eVertexAttributeFormat_4US,     // 4 unsigned shorts
};

enum_serialize_decl(eVertexAttributeFormat);

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
    eVertexAttribute_TileCoord, // for terrain

    eVertexAttribute_COUNT,
    eVertexAttribute_MAX = 16,
};

enum_serialize_decl(eVertexAttribute);

// Get number of component for vertex attribute
// @param attributeFormat: Format identifier
inline unsigned int GetAttributeComponentCount(eVertexAttributeFormat attributeFormat)
{
    switch (attributeFormat)
    {
        case eVertexAttributeFormat_1US: 
            return 1;

        case eVertexAttributeFormat_2US:
        case eVertexAttributeFormat_2F: 
            return 2;

        case eVertexAttributeFormat_3F: 
            return 3;

        case eVertexAttributeFormat_4F:
        case eVertexAttributeFormat_4UB: 
        case eVertexAttributeFormat_4US: 
            return 4;
    }
    cxx_assert(false);
    return 0;
}

// Get vertex attribute size in bytes
// @param attributeFormat: Format identifier
inline unsigned int GetAttributeSizeBytes(eVertexAttributeFormat attributeFormat)
{
    const int componentCount = GetAttributeComponentCount(attributeFormat);
    switch (attributeFormat)
    {
        case eVertexAttributeFormat_1US:
        case eVertexAttributeFormat_2US: 
        case eVertexAttributeFormat_4US: 
            return componentCount * sizeof(unsigned short);

        case eVertexAttributeFormat_2F: 
        case eVertexAttributeFormat_3F: 
        case eVertexAttributeFormat_4F: 
            return componentCount * sizeof(float);

        case eVertexAttributeFormat_4UB: 
            return componentCount * sizeof(unsigned char);
    }
    cxx_assert(false);
    return 0;
}


//////////////////////////////////////////////////////////////////////////

// GpuBuffer usage hint enumeration
enum eBufferUsage
{
    eBufferUsage_Static, // The data store contents will be modified once and used many times
    eBufferUsage_Dynamic, // The data store contents will be modified once and used at most a few times
};

// GpuBuffer content enumeration
enum eBufferTarget
{
    eBufferTarget_Attributes,
    eBufferTarget_Indices,
    eBufferTarget_COUNT // not a valid target
};

// GpuBuffer desired data access enumeration
enum BufferAccessBits
{
    BufferAccess_Read  = (1 << 0),
    BufferAccess_Write = (1 << 1),
    BufferAccess_Unsynchronized = (1 << 2), // client must to guarantee that mapped buffer region is doesn't used by the GPU
    BufferAccess_InvalidateRange = (1 << 3), // have meaning only for range lock
    BufferAccess_InvalidateBuffer = (1 << 4), // orphan whole buffer
    BufferAccess_UnsynchronizedWrite = (BufferAccess_Unsynchronized + BufferAccess_Write),
};

inline BufferAccessBits operator | (BufferAccessBits LHS, BufferAccessBits RHS)
{
    return static_cast<BufferAccessBits>(static_cast<unsigned int>(LHS) + static_cast<unsigned int>(RHS));
}

//////////////////////////////////////////////////////////////////////////

using eBlendingMode = unsigned short;

enum
{
    // blend modes

    RENDER_STATES_BLENDMODE_ALPHA = 0,
    RENDER_STATES_BLENDMODE_ADDITIVE,
    RENDER_STATES_BLENDMODE_ALPHA_ADDITIVE,
    RENDER_STATES_BLENDMODE_MULTIPLY, 
    RENDER_STATES_BLENDMODE_PREMULTIPLIED,
    RENDER_STATES_BLENDMODE_SCREEN,

    // depth dest function

    RENDER_STATES_DEPTHTESTFUNC_ALWAYS = 0, 
    RENDER_STATES_DEPTHTESTFUNC_EQUAL,
    RENDER_STATES_DEPTHTESTFUNC_NOTEQUAL,
    RENDER_STATES_DEPTHTESTFUNC_LESS,
    RENDER_STATES_DEPTHTESTFUNC_GREATER,
    RENDER_STATES_DEPTHTESTFUNC_LEQUAL,
    RENDER_STATES_DEPTHTESTFUNC_GEQUAL,

    // cull face mode

    RENDER_STATES_CULLFACEMODE_FRONT = 0, 
    RENDER_STATES_CULLFACEMODE_BACK,

    // polygon fill mode

    RENDER_STATES_POLYGON_SOLID = 0, 
    RENDER_STATES_POLYGON_WIREFRAME, 
};

// Defines render states
union RenderStates
{
public:
    RenderStates()
        : mBlendingMode(RENDER_STATES_BLENDMODE_ALPHA)
        , mDepthFunc(RENDER_STATES_DEPTHTESTFUNC_LEQUAL)
        , mPolygonFill(RENDER_STATES_POLYGON_SOLID)
        , mCullMode(RENDER_STATES_CULLFACEMODE_BACK)
        , mIsColorWriteEnabled(true)
        , mIsDepthWriteEnabled(true)
        , mIsDepthTestEnabled(true)
        , mIsFaceCullingEnabled(true)
        , mIsAlphaBlendEnabled() // is disabled by default, do not draw dungeon geometry with it
    {}

    // Get default render states for ui drawing
    static RenderStates GetUIStates()
    {
        RenderStates states;
        states.mIsDepthWriteEnabled = false;
        states.mIsDepthTestEnabled = false;
        states.mIsFaceCullingEnabled = false;
        return states.EnableAlphaBlend(RENDER_STATES_BLENDMODE_ALPHA);
    }

    // Enable depth test
    // @param argDepthFunc: Depth test function
    inline RenderStates& EnableDepthTest(int argDepthFunc) 
    {
        mIsDepthTestEnabled = true;
        mDepthFunc = argDepthFunc;
        return *this;
    }

    // Enable alphablend
    // @param argBlendMode: Alpha blend mode
    inline RenderStates& EnableAlphaBlend(int argBlendMode) 
    {
        mIsAlphaBlendEnabled = true;
        mBlendingMode = argBlendMode;
        return *this;
    }

    // Enable culling
    // @param argCullMode: Culling mode
    inline RenderStates& EnableCullFace(int argCullMode) 
    {
        mIsFaceCullingEnabled = true;
        mCullMode = argCullMode;
        return *this;
    }

    inline bool operator == (const RenderStates& renderStates) const { return mSortKey == renderStates.mSortKey; }
    inline bool operator != (const RenderStates& renderStates) const { return mSortKey != renderStates.mSortKey; }
    inline bool operator < (const RenderStates& renderStates) const { return mSortKey < renderStates.mSortKey; }

public:

    struct
    {
        unsigned short mDepthFunc : 4; // DEPTH_TEST_ENABLED
        unsigned short mBlendingMode : 4; // ALPHA_BLEND_ENABLED
        unsigned short mPolygonFill : 4;
        unsigned short mCullMode : 4;
        bool mIsAlphaBlendEnabled : 1;
        bool mIsColorWriteEnabled : 1;
        bool mIsDepthWriteEnabled : 1;
        bool mIsDepthTestEnabled : 1;
        bool mIsFaceCullingEnabled : 1;
    };

    std::uint64_t mSortKey; // union
};

enum
{ 
    Sizeof_RenderStates = sizeof(RenderStates), 
};

//////////////////////////////////////////////////////////////////////////

struct ShaderProgramInputLayout
{
public:
    ShaderProgramInputLayout() = default;
    // Enable vertex attribute
    // @param attributeStream: Attribute stream
    inline void IncludeAttribute(eVertexAttribute attributeStream) 
    {
        if (attributeStream < eVertexAttribute_COUNT) mEnabledAttributes |= (1 << attributeStream);
    }
    // Disable vertex attribute
    // @param attributeStream: Attribute stream
    inline void ExcludeAttribute(eVertexAttribute attributeStream) 
    {
        if (attributeStream < eVertexAttribute_COUNT) mEnabledAttributes &= ~(1 << attributeStream);
    }
    // Test whether vertex attribute enabled
    // @param attributeStream: Attribute stream
    inline bool HasAttribute(eVertexAttribute attributeStream) const 
    {
        return (attributeStream < eVertexAttribute_COUNT) && (mEnabledAttributes & (1 << attributeStream)) > 0;
    }
    unsigned int mEnabledAttributes = 0;
};

inline bool operator == (const ShaderProgramInputLayout& a, const ShaderProgramInputLayout& b) { return a.mEnabledAttributes == b.mEnabledAttributes; }
inline bool operator != (const ShaderProgramInputLayout& a, const ShaderProgramInputLayout& b) { return a.mEnabledAttributes != b.mEnabledAttributes; }

//////////////////////////////////////////////////////////////////////////

struct RenderFrameStatistics
{
public:
    RenderFrameStatistics() = default;
    inline void Clear()
    {
        mNumTrianglesDrawn = 0;
        mNumSwitchTextures = 0;
        mNumSwitchPrograms = 0;
        mNumSwitchVertexBuffers = 0;
        mNumSwitchIndexBuffers = 0;
        mNumDIPs = 0; 
    }

public:
    int mNumTrianglesDrawn = 0;
    int mNumSwitchTextures = 0;
    int mNumSwitchPrograms = 0;
    int mNumSwitchVertexBuffers = 0;
    int mNumSwitchIndexBuffers = 0;
    int mNumDIPs = 0;
};

//////////////////////////////////////////////////////////////////////////

struct Viewport
{
public:
    Viewport() = default;
    inline void Configure(const Rect2D& screenRect)
    {
        mScreenArea = screenRect;
    }
    inline void Configure(const Point2D& screenSize)
    {
        mScreenArea = {0, 0, screenSize.x, screenSize.y};
    }
    inline float GetAspectRatio() const 
    { 
        return (mScreenArea.h > 0) ? ((mScreenArea.w * 1.0f) / (mScreenArea.h * 1.0f)) : 1.0f; 
    }
    inline bool SameAreaSize(const Viewport& rhs) const
    {
        return (mScreenArea.w == rhs.mScreenArea.w) && (mScreenArea.h == rhs.mScreenArea.h);
    }
    // helpers
    inline Point2D GetAreaSize() const { return {mScreenArea.w, mScreenArea.h}; }
    inline Point2D GetAreaPosition() const { return {mScreenArea.x, mScreenArea.y}; }
public:
    Rect2D mScreenArea {};
};

//////////////////////////////////////////////////////////////////////////