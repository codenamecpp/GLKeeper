#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GraphicsDefs.h"

//////////////////////////////////////////////////////////////////////////

// Draw vertex 3d
struct Vertex3D
{
public:
    glm::vec3 mPosition; // 12 bytes
    glm::vec3 mNormal; // 12 bytes
    glm::vec2 mTexcoord; // 8 bytes
    Color32 mColor; // 4 bytes
};

const unsigned int Sizeof_Vertex3D = sizeof(Vertex3D);

//////////////////////////////////////////////////////////////////////////

struct TerrainVertex3D
{
public:
    TerrainVertex3D() = default;
    TerrainVertex3D(const Vertex3D& sourcevertex, unsigned short tilex, unsigned short tiley)
        : mPosition(sourcevertex.mPosition)
        , mNormal(sourcevertex.mNormal)
        , mTexcoord(sourcevertex.mTexcoord)
        , mTileX(tilex)
        , mTileY(tiley)
    {
    }
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

const unsigned int Sizeof_TerrainVertex = sizeof(TerrainVertex3D);

//////////////////////////////////////////////////////////////////////////

// Draw vertex 2d
struct Vertex2D
{
public:
    glm::vec2 mPosition; // 12 bytes
    glm::vec2 mTexcoord; // 8 bytes
    Color32 mColor; // 4 bytes
};

const unsigned int Sizeof_Vertex2D = sizeof(Vertex2D);

//////////////////////////////////////////////////////////////////////////

// Draw vertex debug
struct DebugVertex3D
{
public:
    glm::vec3 mPosition; // 12 bytes
    Color32 mColor; // 4 bytes
};

const unsigned int Sizeof_DebugVertex3D = sizeof(DebugVertex3D);

//////////////////////////////////////////////////////////////////////////

// render vertex 3d water lava mesh
struct WaterLavaVertex
{
public:
    glm::vec3 mPosition; // 12 bytes
    glm::vec2 mTexcoord; // 8 bytes
};

const unsigned int Sizeof_WaterLavaVertex = sizeof(WaterLavaVertex);

//////////////////////////////////////////////////////////////////////////

// defines vertex attributes streams

//////////////////////////////////////////////////////////////////////////

struct VertexFormat
{
public:
    //////////////////////////////////////////////////////////////////////////
    struct SingleAttribute
    {
    public:
        SingleAttribute() = default;
    public:
        eVertexAttributeFormat mFormat = eVertexAttributeFormat_Null;
        unsigned int mDataOffset = 0;
        // attribute normalization - opengl specific
        // if set to true, it indicates that values stored in an integer format are 
        // to be mapped to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed and converted to floating point
        bool mNormalized = false;
    };
    //////////////////////////////////////////////////////////////////////////
public:
    inline void ConfigureWith(const VertexFormat& rhs)
    {
        for (int iroller = 0; iroller < eVertexAttribute_COUNT; ++iroller)
        {
            mAttributes[iroller] = rhs.mAttributes[iroller];
        }
        mDataStride = rhs.mDataStride;
        mBaseOffset = rhs.mBaseOffset;
    }
    // Enable attribute
    // @param attribute: Attribute identifier
    // @param dataOffset: Attribute data offset in bytes within buffer
    inline void ConfigureAttribute(eVertexAttribute attribute, eVertexAttributeFormat attributeFormat, unsigned int dataOffset)
    {
        cxx_assert(attribute < eVertexAttribute_COUNT);
        cxx_assert(attributeFormat != eVertexAttributeFormat_Null);
        mAttributes[attribute].mDataOffset = dataOffset;
        mAttributes[attribute].mFormat = attributeFormat;
        mAttributes[attribute].mNormalized = false;
    }
    inline void SetAttributeNormalized(eVertexAttribute attribute, bool isNormalized = true)
    {
        cxx_assert(attribute < eVertexAttribute_COUNT);
        mAttributes[attribute].mNormalized = isNormalized;
    }
    // Set data stride common to all attributes
    // @param dataStride: Stride in bytes
    inline void SetDataStride(unsigned int dataStride) { mDataStride = dataStride; }
    inline void SetBaseOffset(unsigned int baseOffset) { mBaseOffset = baseOffset; }
public:
    SingleAttribute mAttributes[eVertexAttribute_COUNT];
    unsigned int mDataStride = 0; // common to all attributes
    unsigned int mBaseOffset = 0; // additional offset in bytes within source vertex buffer, affects on all attribues
};

//////////////////////////////////////////////////////////////////////////

// standard engine vertex definition
struct Vertex3D_Format: public VertexFormat
{
public:
    using TVertexType = Vertex3D;
    Vertex3D_Format()
    {
        Setup();
    }
    // Get definition instance
    static const Vertex3D_Format& Get() 
    { 
        static const Vertex3D_Format sDefinition; 
        return sDefinition; 
    }
    // Initialzie definition
    inline void Setup()
    {
        this->SetDataStride(Sizeof_Vertex3D);
        this->ConfigureAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->ConfigureAttribute(eVertexAttribute_Normal0, eVertexAttributeFormat_3F, offsetof(TVertexType, mNormal));
        this->ConfigureAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
        this->ConfigureAttribute(eVertexAttribute_Color0, eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));
        this->SetAttributeNormalized(eVertexAttribute_Color0);
    }
};

//////////////////////////////////////////////////////////////////////////

// ui vertex definition
struct Vertex2D_Format: public VertexFormat
{
public:
    using TVertexType = Vertex2D;
    Vertex2D_Format()
    {
        Setup();
    }
    // Get definition instance
    static const Vertex2D_Format& Get() 
    { 
        static const Vertex2D_Format sDefinition; 
        return sDefinition; 
    }
    inline void Setup()
    {
        this->SetDataStride(Sizeof_Vertex2D);
        this->ConfigureAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_2F, offsetof(TVertexType, mPosition));
        this->ConfigureAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
        this->ConfigureAttribute(eVertexAttribute_Color0, eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));
        this->SetAttributeNormalized(eVertexAttribute_Color0);
    }
};

//////////////////////////////////////////////////////////////////////////

// debug vertex definition
struct Vertex3D_Debug_Definition: public VertexFormat
{
public:
    using TVertexType = DebugVertex3D;
    Vertex3D_Debug_Definition()
    {
        Setup();
    }
    // Get definition instance
    static const Vertex3D_Debug_Definition& Get() 
    { 
        static const Vertex3D_Debug_Definition sDefinition; 
        return sDefinition; 
    }
    inline void Setup()
    {
        this->SetDataStride(Sizeof_DebugVertex3D);
        this->ConfigureAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->ConfigureAttribute(eVertexAttribute_Color0, eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));
        this->SetAttributeNormalized(eVertexAttribute_Color0);
    }
};

//////////////////////////////////////////////////////////////////////////

// morph/keyframe animation vertex definition
// no color data stored!
struct AnimVertex3D_Format: public VertexFormat
{
public:
    // does not declare TVertexType due to specific attributes layout
    AnimVertex3D_Format()
    {
        this->SetDataStride(0);
    }

    // Initialzie definition

    // attributes layout:
    // submesh #0 - [texture coords] [positions of all animation frames] [normals of all animation frames]
    // submesh #1 - [texture coords] [positions of all animation frames] [normals of all animation frames]
    // and so on

    enum 
    { 
        Sizeof_Texcoord = sizeof(glm::vec2),
        Sizeof_Position = sizeof(glm::vec3),
        Sizeof_Normal = sizeof(glm::vec3),
    };

    inline void Setup(int dataOffset, int numVertsPerFrame, int numFrames, int frame0, int frame1)
    {
        this->SetBaseOffset(dataOffset);
        this->ConfigureAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, 0);
        // frame 0
        this->ConfigureAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, 
            (numVertsPerFrame * Sizeof_Texcoord) + 
            (numVertsPerFrame * Sizeof_Position * frame0));
        this->ConfigureAttribute(eVertexAttribute_Normal0, eVertexAttributeFormat_3F, 
            (numVertsPerFrame * Sizeof_Texcoord) + 
            (numVertsPerFrame * Sizeof_Position * numFrames) + 
            (numVertsPerFrame * Sizeof_Normal * frame0));
        // frame 1
        this->ConfigureAttribute(eVertexAttribute_Position1, eVertexAttributeFormat_3F, 
            (numVertsPerFrame * Sizeof_Texcoord) + 
            (numVertsPerFrame * Sizeof_Position * frame1));
        this->ConfigureAttribute(eVertexAttribute_Normal1, eVertexAttributeFormat_3F, 
            (numVertsPerFrame * Sizeof_Texcoord) + 
            (numVertsPerFrame * Sizeof_Position * numFrames) + 
            (numVertsPerFrame * Sizeof_Normal * frame1));
    }
};

//////////////////////////////////////////////////////////////////////////

// terrain vertex definition
struct TerrainVertex3D_Format: public VertexFormat
{
public:
    using TVertexType = TerrainVertex3D;
    TerrainVertex3D_Format()
    {
        Setup();
    }
    // Get definition instance
    static const TerrainVertex3D_Format& Get() 
    { 
        static const TerrainVertex3D_Format sDefinition; 
        return sDefinition; 
    }
    // Initialzie definition
    inline void Setup()
    {
        this->SetDataStride(Sizeof_TerrainVertex);
        this->ConfigureAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->ConfigureAttribute(eVertexAttribute_Normal0, eVertexAttributeFormat_3F, offsetof(TVertexType, mNormal));
        this->ConfigureAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
        this->ConfigureAttribute(eVertexAttribute_TileCoord, eVertexAttributeFormat_2US, offsetof(TVertexType, mTileCoord));
    }
};

//////////////////////////////////////////////////////////////////////////

// water or lava vertex definition
struct WaterLavaVertex3D_Format: public VertexFormat
{
public:
    WaterLavaVertex3D_Format()
    {
        Setup();
    }
    // get definition instance
    static const WaterLavaVertex3D_Format& Get() 
    { 
        static const WaterLavaVertex3D_Format sDefinition; 
        return sDefinition; 
    }
    using TVertexType = WaterLavaVertex;
    // initialize definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_WaterLavaVertex;
        this->ConfigureAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->ConfigureAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
    }
};

