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
    // initialzie definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_Vertex3D;
        this->SetAttribute(eVertexAttribute_Position0,  eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Normal0,    eVertexAttributeFormat_3F, offsetof(TVertexType, mNormal));
        this->SetAttribute(eVertexAttribute_Texcoord0,  eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
        this->SetAttribute(eVertexAttribute_Color0,     eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));
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
    // initialzie definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_Vertex2D;
        this->SetAttribute(eVertexAttribute_Texcoord0,  eVertexAttributeFormat_2F, offsetof(TVertexType, mTexcoord));
        this->SetAttribute(eVertexAttribute_Position0,  eVertexAttributeFormat_2F, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Color0,     eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));
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
    // initialzie definition
    inline void Setup()
    {
        this->mDataStride = Sizeof_Vertex3D_Debug;
        this->SetAttribute(eVertexAttribute_Position0,  eVertexAttributeFormat_3F, offsetof(TVertexType, mPosition));
        this->SetAttribute(eVertexAttribute_Color0,     eVertexAttributeFormat_4UB, offsetof(TVertexType, mColor));
    }
};
