#include "stdafx.h"
#include "GpuProgram.h"
#include "OpenGLDefs.h"

//////////////////////////////////////////////////////////////////////////

const char* gGLSL_version_string = "#version 330 core\n";

//////////////////////////////////////////////////////////////////////////

static char ShaderMessageBuffer[4096];

enum 
{ 
    MAX_SHADER_INFO_LEN = CountOf(ShaderMessageBuffer) - 1 
};

//////////////////////////////////////////////////////////////////////////

GpuProgram* GpuProgram::sCurrentProgram = nullptr;

//////////////////////////////////////////////////////////////////////////

// Internal helper class
struct GpuShader
{
public:
    GpuShader(GLenum aShaderType)
        : mTarget(aShaderType)
        , mHandle()
    {
        mHandle = ::glCreateShader(aShaderType);
        glCheckErrors();
    }
    ~GpuShader() 
    {
        ::glDeleteShader(mHandle);
        glCheckErrors();
    }
    // compile shader
    bool Compile(const char** szSource, int numStrings) 
    {
        GLint resultGL;
        GLint length;

        ::glShaderSource(mHandle, numStrings, szSource, nullptr);
        glCheckErrors();

        ::glCompileShader(mHandle);
        glCheckErrors();

        ::glGetShaderiv(mHandle, GL_COMPILE_STATUS, &resultGL);
        glCheckErrors();

        ::glGetShaderiv(mHandle, GL_INFO_LOG_LENGTH, &length);
        glCheckErrors();

        if (length > 1)
        {
            ::glGetShaderInfoLog(mHandle, MAX_SHADER_INFO_LEN, nullptr, ShaderMessageBuffer);
            glCheckErrors();

            gConsole.LogMessage(eLogLevel_Info, "Shader compilation message: '%s'", ShaderMessageBuffer);
        }
        return resultGL != GL_FALSE;
    };
public:
    GLenum mTarget;
    GLuint mHandle;
};

//////////////////////////////////////////////////////////////////////////

GpuProgram::GpuProgram()
    : mResourceHandle()
    , mInputLayout()
{
    mResourceHandle = ::glCreateProgram();
    glCheckErrors();

    // clear all locations
    for (GpuVariableLocation& location: mAttributes) { location = -1; }
    for (GpuVariableLocation& location: mSamplers) { location = -1; }
}

GpuProgram::~GpuProgram()
{
    // set unbound
    if (this == sCurrentProgram)
    {
        sCurrentProgram = nullptr;
    }

    ::glDeleteProgram(mResourceHandle);
    glCheckErrors();
}

bool GpuProgram::IsProgramBound() const
{
    return sCurrentProgram == this;
}

bool GpuProgram::IsProgramCompiled() const
{
    return mInputLayout.mEnabledAttributes > 0;
}

bool GpuProgram::BindAttribute(eVertexAttribute attributeIdentifier, const char* attributeName)
{
    mInputLayout.IncludeAttribute(attributeIdentifier);
    mAttributes[attributeIdentifier] = QueryAttributeLocation(attributeName);
    if (mAttributes[attributeIdentifier] == GpuLocation_Null)
    {
        cxx_assert(false);
        return false;
    }
    return true;
}

void GpuProgram::SetUniform(GpuVariableLocation uniformLocation, float param0)
{
    cxx_assert(IsProgramCompiled());
    cxx_assert(uniformLocation != GpuLocation_Null);
    ::glProgramUniform1f(mResourceHandle, uniformLocation, param0);
    glCheckErrors();
}

void GpuProgram::SetUniform(GpuVariableLocation uniformLocation, float param0, float param1)
{
    cxx_assert(IsProgramCompiled());
    cxx_assert(uniformLocation != GpuLocation_Null);
    ::glProgramUniform2f(mResourceHandle, uniformLocation, param0, param1);
    glCheckErrors();
}

void GpuProgram::SetUniform(GpuVariableLocation uniformLocation, float param0, float param1, float param2)
{
    cxx_assert(IsProgramCompiled());
    cxx_assert(uniformLocation != GpuLocation_Null);
    ::glProgramUniform3f(mResourceHandle, uniformLocation, param0, param1, param2);
    glCheckErrors();
}

void GpuProgram::SetUniform(GpuVariableLocation uniformLocation, int param0)
{
    cxx_assert(IsProgramCompiled());
    cxx_assert(uniformLocation != GpuLocation_Null);
    ::glProgramUniform1i(mResourceHandle, uniformLocation, param0);
    glCheckErrors();
}

void GpuProgram::SetUniform(GpuVariableLocation uniformLocation, const glm::vec2& floatVector2)
{
    cxx_assert(IsProgramCompiled());
    cxx_assert(uniformLocation != GpuLocation_Null);
    ::glProgramUniform2fv(mResourceHandle, uniformLocation, 1, &floatVector2.x);
    glCheckErrors();
}

void GpuProgram::SetUniform(GpuVariableLocation uniformLocation, const glm::vec3& floatVector3)
{
    cxx_assert(IsProgramCompiled());
    cxx_assert(uniformLocation != GpuLocation_Null);
    ::glProgramUniform3fv(mResourceHandle, uniformLocation, 1, &floatVector3.x);
    glCheckErrors();
}

void GpuProgram::SetUniform(GpuVariableLocation uniformLocation, const glm::vec4& floatVector4)
{
    cxx_assert(IsProgramCompiled());
    cxx_assert(uniformLocation != GpuLocation_Null);
    ::glProgramUniform4fv(mResourceHandle, uniformLocation, 1, &floatVector4.x);
    glCheckErrors();
}

void GpuProgram::SetUniform(GpuVariableLocation uniformLocation, const glm::mat3& floatMatrix3)
{
    cxx_assert(IsProgramCompiled());
    cxx_assert(uniformLocation != GpuLocation_Null);
    ::glProgramUniformMatrix3fv(mResourceHandle, uniformLocation, 1, GL_FALSE, &floatMatrix3[0][0]);
    glCheckErrors();
}

void GpuProgram::SetUniform(GpuVariableLocation uniformLocation, const glm::mat4& floatMatrix4)
{
    cxx_assert(IsProgramCompiled());
    cxx_assert(uniformLocation != GpuLocation_Null);
    ::glProgramUniformMatrix4fv(mResourceHandle, uniformLocation, 1, GL_FALSE, &floatMatrix4[0][0]);
    glCheckErrors();
}

bool GpuProgram::CompileShader(const std::string& vertShaderSrc, const std::string& fragShaderSrc)
{
    // set unbound
    if (this == sCurrentProgram)
    {
        sCurrentProgram = nullptr;
    }

    bool isSuccessed = false;
    if (IsProgramCompiled())
    {
        // create temporary program
        GLuint programHandleGL = ::glCreateProgram();
        glCheckErrors();

        isSuccessed = CompileShader(programHandleGL, vertShaderSrc.c_str(), fragShaderSrc.c_str());
        if (!isSuccessed)
        {
            // destroy temporary program
            ::glDeleteProgram(programHandleGL);
            glCheckErrors();
        }
        else
        {
            // destroy old program object
            ::glDeleteProgram(mResourceHandle);
            glCheckErrors();
            mResourceHandle = programHandleGL;
        }
    }
    else
    {
        isSuccessed = CompileShader(mResourceHandle, vertShaderSrc.c_str(), fragShaderSrc.c_str());
    }

    if (!isSuccessed)
        return false;

    // clear old program data
    mInputLayout.mEnabledAttributes = 0;

    for (GpuVariableLocation& location: mAttributes) { location = -1; }
    for (GpuVariableLocation& location: mSamplers) { location = -1; }

    // query attributes
    for (int iattribute = 0; iattribute < eVertexAttribute_COUNT; ++iattribute)
    {
        eVertexAttribute vertexAttribute = (eVertexAttribute) iattribute;
        mAttributes[iattribute] = ::glGetAttribLocation(mResourceHandle, cxx::enum_to_string(vertexAttribute));
        if (mAttributes[iattribute] != -1)
        {
            mInputLayout.IncludeAttribute(vertexAttribute);
        }        
    }

    // query samplers
    for (int isampler = 0; isampler < eTextureUnit_COUNT; ++isampler)
    {
        GLint ilocation = ::glGetUniformLocation(mResourceHandle, ToString((eTextureUnit) isampler));
        glCheckErrors();
        if (ilocation > -1)
        {
            mSamplers[isampler] = ilocation;
            // bind sampler to default slot
            ::glProgramUniform1i(mResourceHandle, ilocation, isampler);
            glCheckErrors();
        }
    }

    return true;
}

bool GpuProgram::CompileShader(GpuResourceHandle targetHandle, const char* vertShaderSrc, const char* fragShaderSrc)
{
    // source strings
    const char* vertSource[] = { gGLSL_version_string, vertShaderSrc };
    const char* fragSource[] = { gGLSL_version_string, fragShaderSrc };

    // compile shaders
    GpuShader vertexShader (GL_VERTEX_SHADER);
    if (!vertexShader.Compile(vertSource, CountOf(vertSource)))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Vertex shader compilation failed");
        return false;
    }

    GpuShader fragmentShader (GL_FRAGMENT_SHADER);
    if (!fragmentShader.Compile(fragSource, CountOf(fragSource)))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Fragment shader compilation failed");
        return false;
    }

    // linking program
    GLint length;
    GLint linkResultGL;
    
    ::glAttachShader(targetHandle, vertexShader.mHandle);
    glCheckErrors();

    ::glAttachShader(targetHandle, fragmentShader.mHandle);
    glCheckErrors();

    ::glLinkProgram(targetHandle);
    glCheckErrors();

    ::glGetProgramiv(targetHandle, GL_LINK_STATUS, &linkResultGL);
    glCheckErrors();

    ::glGetProgramiv(targetHandle, GL_INFO_LOG_LENGTH, &length);
    glCheckErrors();

    if (length > 1) 
    {
        ::glGetProgramInfoLog(targetHandle, MAX_SHADER_INFO_LEN, nullptr, ShaderMessageBuffer);
        glCheckErrors();
        gConsole.LogMessage(eLogLevel_Info, "Program linkage message: '%s'", ShaderMessageBuffer);
    }

    if (linkResultGL == GL_FALSE)
    {
        gConsole.LogMessage(eLogLevel_Warning, "Render program linkage error");
        return false;
    }
    return true;
}

GpuVariableLocation GpuProgram::QueryUniformLocation(const char* constantName) const
{
    GpuVariableLocation outLocation = ::glGetUniformLocation(mResourceHandle, constantName);
    glCheckErrors();

    return outLocation;
}

GpuVariableLocation GpuProgram::QueryAttributeLocation(const char* attributeName) const
{
    GpuVariableLocation outLocation = ::glGetAttribLocation(mResourceHandle, attributeName);
    glCheckErrors();

    return outLocation;
}
