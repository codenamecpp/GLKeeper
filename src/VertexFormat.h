#pragma once

#include "GraphicsDefs.h"

// defines vertex attributes streams
struct VertexFormat
{
public:
    VertexFormat() = default;

    // enable attribute or modify data offset for enabled attribute
    // @param attribute: Attribute identifier
    // @param vertexFormat: Attribute format
    // @param dataOffset: Attribute data offset in bytes within buffer
    inline void SetAttribute(eVertexAttribute attribute, eVertexAttributeFormat vertexFormat, unsigned int dataOffset)
    {
        debug_assert(attribute < eVertexAttribute_COUNT);
        mAttributes[attribute].mDataOffset = dataOffset;

        debug_assert(vertexFormat != eVertexAttributeFormat_Unknown);
        mAttributes[attribute].mFormat = vertexFormat;

        mAttributes[attribute].mNormalized = false;
    }

    inline void SetAttributeNormalized(eVertexAttribute attribute, bool isNormalized)
    {
        mAttributes[attribute].mNormalized = isNormalized;
    }

public:
    struct SingleAttribute
    {
    public:
        SingleAttribute() = default;
    public:
        eVertexAttributeFormat mFormat = eVertexAttributeFormat_Unknown;
        unsigned int mDataOffset = 0;

        // attribute normalization - opengl specific
        // if set to true, it indicates that values stored in an integer format are 
        // to be mapped to the range [-1,1] (for signed values) or [0,1] (for unsigned values) when they are accessed and converted to floating point
        bool mNormalized = false;
    };

    SingleAttribute mAttributes[eVertexAttribute_COUNT];
    unsigned int mDataStride = 0; // common to all attributes
    unsigned int mBaseOffset = 0; // additional offset in bytes within source vertex buffer, affects on all attribues
};

//////////////////////////////////////////////////////////////////////////

// standard engine vertex definition
struct Vertex3D_Format: public VertexFormat
{
public:
    Vertex3D_Format()
    {
        Setup();
    }
    // get format definition
    static const Vertex3D_Format& Get() 
    { 
        static const Vertex3D_Format sDefinition; 
        return sDefinition; 
    }
    using TVertexType = Vertex3D;
    // initialize definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_Vertex3D;
        this->SetAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
        this->SetAttribute(eVertexAttribute_Normal0, eVertexAttributeFormat_3F, offsetof(TVertexType, mNormal));
        this->SetAttribute(eVertexAttribute_Color0, eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));

        this->SetAttributeNormalized(eVertexAttribute_Color0, true);
    }
};

//////////////////////////////////////////////////////////////////////////

// ui vertex definition
struct Vertex2D_Format: public VertexFormat
{
public:
    Vertex2D_Format()
    {
        Setup();
    }
    // get format definition
    static const Vertex2D_Format& Get() 
    { 
        static const Vertex2D_Format sDefinition; 
        return sDefinition; 
    }
    using TVertexType = Vertex2D;
    // initialize definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_Vertex2D;
        this->SetAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
        this->SetAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_2F, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Color0, eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));

        this->SetAttributeNormalized(eVertexAttribute_Color0, true);
    }
};

//////////////////////////////////////////////////////////////////////////

// debug vertex definition
struct Vertex3D_Debug_Format: public VertexFormat
{
public:
    Vertex3D_Debug_Format()
    {
        Setup();
    }
    // get definition instance
    static const Vertex3D_Debug_Format& Get() 
    { 
        static const Vertex3D_Debug_Format sDefinition; 
        return sDefinition; 
    }
    using TVertexType = Vertex3D_Debug;
    // initialize definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_Vertex3D_Debug;
        this->SetAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Color0, eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));

        this->SetAttributeNormalized(eVertexAttribute_Color0, true);
    }
};

//////////////////////////////////////////////////////////////////////////

// terrain vertex definition
struct Vertex3D_Terrain_Format: public VertexFormat
{
public:
    Vertex3D_Terrain_Format()
    {
        Setup();
    }
    // get definition instance
    static const Vertex3D_Terrain_Format& Get() 
    { 
        static const Vertex3D_Terrain_Format sDefinition; 
        return sDefinition; 
    }
    using TVertexType = Vertex3D_Terrain;
    // initialize definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_Vertex3D_Terrain;
        this->SetAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
        this->SetAttribute(eVertexAttribute_Normal0, eVertexAttributeFormat_3F, offsetof(TVertexType, mNormal));
        this->SetAttribute(eVertexAttribute_TerrainTilePosition, eVertexAttributeFormat_2US, offsetof(TVertexType, mTileCoord));
    }
};

//////////////////////////////////////////////////////////////////////////

// water or lava vertex definition
struct Vertex3D_WaterLava_Format: public VertexFormat
{
public:
    Vertex3D_WaterLava_Format()
    {
        Setup();
    }
    // get definition instance
    static const Vertex3D_WaterLava_Format& Get() 
    { 
        static const Vertex3D_WaterLava_Format sDefinition; 
        return sDefinition; 
    }
    using TVertexType = Vertex3D_WaterLava;
    // initialize definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_Vertex3D_WaterLava;
        this->SetAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
    }
};

//////////////////////////////////////////////////////////////////////////

// morph/keyframe animation vertex definition
// no color data stored!
struct Vertex3D_Anim_Format: public VertexFormat
{
public:
    // does not declare TVertexType due to specific attributes layout

    Vertex3D_Anim_Format()
    {
        this->mDataStride = 0;
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
        this->SetAttribute(eVertexAttribute_Texcoord0, eVertexAttributeFormat_2F, dataOffset);

        // frame 0
        this->SetAttribute(eVertexAttribute_Position0, eVertexAttributeFormat_3F,
            (dataOffset + numVertsPerFrame * Sizeof_Texcoord) + 
            (numVertsPerFrame * Sizeof_Position * frame0));

        this->SetAttribute(eVertexAttribute_Normal0, eVertexAttributeFormat_3F, 
            (dataOffset + numVertsPerFrame * Sizeof_Texcoord) + 
            (numVertsPerFrame * Sizeof_Position * numFrames) + 
            (numVertsPerFrame * Sizeof_Normal * frame0));

        // frame 1
        this->SetAttribute(eVertexAttribute_Position1, eVertexAttributeFormat_3F, 
            (dataOffset + numVertsPerFrame * Sizeof_Texcoord) + 
            (numVertsPerFrame * Sizeof_Position * frame1));

        this->SetAttribute(eVertexAttribute_Normal1, eVertexAttributeFormat_3F, 
            (dataOffset + numVertsPerFrame * Sizeof_Texcoord) + 
            (numVertsPerFrame * Sizeof_Position * numFrames) + 
            (numVertsPerFrame * Sizeof_Normal * frame1));
    }
};