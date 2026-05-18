#pragma once

#include "GraphicsDefs.h"

//////////////////////////////////////////////////////////////////////////

// WARNING: THIS IS PART OF IMPLEMENTATION DETAILS
// DO NOT USE ANY DECLARATIONS, FUNCTIONS OR CLASSES LISTED HERE!

//////////////////////////////////////////////////////////////////////////

#define BUFFER_OFFSET(offset) ((unsigned char*)0 + (offset))

// target opengl version code
#define OPENGL_CONTEXT_MAJOR_VERSION 3
#define OPENGL_CONTEXT_MINOR_VERSION 2

// reset current opengl error code
inline void ClearGLError()
{
    for (GLenum glErrorCode = ::glGetError(); glErrorCode != GL_NO_ERROR; glErrorCode = ::glGetError()) 
    {}
}

#ifdef _DEBUG
    #define glCheckErrors()\
        {\
            GLenum errcode = ::glGetError();\
            if (errcode != GL_NO_ERROR)\
            {\
                gConsole.LogMessage(eLogLevel_Error, "OpenGL error detected in %s, code 0x%04X", __FUNCTION__, errcode);\
                cxx_assert(false); \
            }\
        }
#else
    #define glCheckErrors()
#endif


inline GLenum ToGLEnum(ePrimitiveType primitiveType)
{
    switch (primitiveType)
    {
        case ePrimitiveType_TriangleStrip: return GL_TRIANGLE_STRIP;
        case ePrimitiveType_Points: return GL_POINTS;
        case ePrimitiveType_Lines: return GL_LINES;
        case ePrimitiveType_LineLoop: return GL_LINE_LOOP;
        case ePrimitiveType_Triangles: return GL_TRIANGLES;
        case ePrimitiveType_TriangleFan: return GL_TRIANGLE_FAN;
        default:
        {
            cxx_assert(false);
        }
    }
    return GL_TRIANGLES;
}

inline GLenum EnumToGL(eIndicesType indicesType)
{
    switch (indicesType)
    {
        case eIndicesType_i16: return GL_UNSIGNED_SHORT;
        case eIndicesType_i32: return GL_UNSIGNED_INT;
    }
    cxx_assert(false);
    return GL_UNSIGNED_INT;
}

inline GLenum ToGLEnum(eBufferUsage usage)
{
    switch (usage)
    {
        case eBufferUsage_Static: return GL_STATIC_DRAW;
        case eBufferUsage_Dynamic: return GL_STREAM_DRAW;
        default:
        {
            cxx_assert(false);
        }
    }
    return GL_STATIC_DRAW;
}

inline GLenum ToGLEnum(eBufferTarget bufferTarget)
{
    if (bufferTarget == eBufferTarget_Attributes)
        return GL_ARRAY_BUFFER;

    if (bufferTarget == eBufferTarget_Indices)
        return GL_ELEMENT_ARRAY_BUFFER;

    cxx_assert(false);
    return GL_ARRAY_BUFFER;
}

inline GLuint GetTextureInputFormatGL(ePixelFormat textureFormat)
{
    switch (textureFormat)
    {
        case ePixelFormat_R8: return GL_RED;
        case ePixelFormat_R8_G8: return GL_RG;
        case ePixelFormat_RGB8: return GL_RGB;
        case ePixelFormat_RGBA8: return GL_RGBA;
        case ePixelFormat_RGBA8UI:
        case ePixelFormat_R16UI: 
        case ePixelFormat_R8UI:
            return GL_RED_INTEGER;
    }
    cxx_assert(false);
    return 0;
}

inline GLint GetTextureInternalFormatGL(ePixelFormat textureFormat)
{
    switch (textureFormat)
    {
        case ePixelFormat_R8: return GL_R8;
        case ePixelFormat_R8_G8: return GL_RG8;
        case ePixelFormat_RGB8: return GL_RGB8;
        case ePixelFormat_RGBA8: return GL_RGBA8;
        case ePixelFormat_R16UI: return GL_R16UI;
        case ePixelFormat_R8UI: return GL_R8UI;
        case ePixelFormat_RGBA8UI: return GL_RGBA8UI;
    }
    cxx_assert(false);
    return 0;
}

inline GLenum GetTextureDataTypeGL(ePixelFormat textureFormat)
{
    switch (textureFormat)
    {
        case ePixelFormat_R8:
        case ePixelFormat_R8_G8:
        case ePixelFormat_RGB8:
        case ePixelFormat_RGBA8: 
        case ePixelFormat_R8UI:
        case ePixelFormat_RGBA8UI:
            return GL_UNSIGNED_BYTE;

        case ePixelFormat_R16UI: 
            return GL_UNSIGNED_SHORT;

        default: break;
    }
    cxx_assert(false);
    return 0;
}

inline GLenum GetAttributeDataTypeGL(eVertexAttributeFormat attributeFormat)
{
    switch (attributeFormat)
    {
        case eVertexAttributeFormat_2F:
        case eVertexAttributeFormat_3F:
        case eVertexAttributeFormat_4F: 
            return GL_FLOAT;

        case eVertexAttributeFormat_4UB: 
            return GL_UNSIGNED_BYTE;

        case eVertexAttributeFormat_1US:
        case eVertexAttributeFormat_2US:
        case eVertexAttributeFormat_4US: 
            return GL_UNSIGNED_SHORT;
    }
    cxx_assert(false);
    return GL_UNSIGNED_BYTE;
}