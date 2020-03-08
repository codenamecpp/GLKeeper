#include "pch.h"
#include "GpuProgram.h"
#include "GraphicsDeviceContext.h"
#include "OpenGL_Defs.h"

static const char* gGLSL_version_string = "#version 330 core\n";
static const char* gGLSL_vertex_shader_string = "#define VERTEX_SHADER\n";
static const char* gGLSL_fragment_shader_string = "#define FRAGMENT_SHADER\n";
const int MaxShaderInfoLength = 2048;
static char ShaderMessageBuffer[MaxShaderInfoLength + 1];

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
        glCheckError();
    }
    ~GpuShader() 
    {
        ::glDeleteShader(mHandle);
        glCheckError();
    }
    // compile shader
    bool Compile(const char** szSource, int numStrings) 
    {
        GLint resultGL;
        GLint length;

        ::glShaderSource(mHandle, numStrings, szSource, nullptr);
        glCheckError();

        ::glCompileShader(mHandle);
        glCheckError();

        ::glGetShaderiv(mHandle, GL_COMPILE_STATUS, &resultGL);
        glCheckError();

        ::glGetShaderiv(mHandle, GL_INFO_LOG_LENGTH, &length);
        glCheckError();

        if (length > 1)
        {
            ::glGetShaderInfoLog(mHandle, MaxShaderInfoLength, nullptr, ShaderMessageBuffer);
            glCheckError();

            gConsole.LogMessage(eLogMessage_Info, "Shader compilation message: '%s'", ShaderMessageBuffer);
        }
        return resultGL != GL_FALSE;
    };
public:
    GLenum mTarget;
    GLuint mHandle;
};

//////////////////////////////////////////////////////////////////////////

GpuProgram::GpuProgram(GraphicsDeviceContext& graphicsContext)
    : mResourceHandle(GpuProgramHandle_NULL)
    , mInputLayout()
    , mGraphicsContext(graphicsContext)
{
    // clear all locations
    for (GpuVariableLocation& location: mAttributes) { location = GpuVariable_NULL; }
    for (GpuVariableLocation& location: mSamplers) { location = GpuVariable_NULL; }
}

GpuProgram::~GpuProgram()
{
    FreeProgram();
}

bool GpuProgram::IsProgramBound() const
{
    return this == mGraphicsContext.mCurrentProgram;
}

bool GpuProgram::IsProgramCompiled() const
{
    return mResourceHandle != GpuProgramHandle_NULL;
}

bool GpuProgram::IsProgramConfigured() const
{
    return mInputLayout.mEnabledAttributes > 0;
}

bool GpuProgram::BindAttribute(eVertexAttribute attributeIdentifier, const char* attributeName)
{
    mInputLayout.IncludeAttribute(attributeIdentifier);
    mAttributes[attributeIdentifier] = QueryAttributeLocation(attributeName);
    if (mAttributes[attributeIdentifier] == GpuVariable_NULL)
    {
        debug_assert(false);
        return false;
    }
    return true;
}

void GpuProgram::SetUniformParam(GpuVariableLocation constantLocation, float param0)
{
    debug_assert(constantLocation != GpuVariable_NULL);
    ::glProgramUniform1f(mResourceHandle, constantLocation, param0);
    glCheckError();
}

void GpuProgram::SetUniformParam(GpuVariableLocation constantLocation, float param0, float param1)
{
    debug_assert(constantLocation != GpuVariable_NULL);
    ::glProgramUniform2f(mResourceHandle, constantLocation, param0, param1);
    glCheckError();
}

void GpuProgram::SetUniformParam(GpuVariableLocation constantLocation, float param0, float param1, float param2)
{
    debug_assert(constantLocation != GpuVariable_NULL);
    ::glProgramUniform3f(mResourceHandle, constantLocation, param0, param1, param2);
    glCheckError();
}

void GpuProgram::SetUniformParam(GpuVariableLocation constantLocation, int param0)
{
    debug_assert(constantLocation != GpuVariable_NULL);
    ::glProgramUniform1i(mResourceHandle, constantLocation, param0);
    glCheckError();
}

void GpuProgram::SetUniformParam(GpuVariableLocation constantLocation, const glm::vec2& floatVector2)
{
    debug_assert(constantLocation != GpuVariable_NULL);
    ::glProgramUniform2fv(mResourceHandle, constantLocation, 1, &floatVector2.x);
    glCheckError();
}

void GpuProgram::SetUniformParam(GpuVariableLocation constantLocation, const glm::vec3& floatVector3)
{
    debug_assert(constantLocation != GpuVariable_NULL);
    ::glProgramUniform3fv(mResourceHandle, constantLocation, 1, &floatVector3.x);
    glCheckError();
}

void GpuProgram::SetUniformParam(GpuVariableLocation constantLocation, const glm::vec4& floatVector4)
{
    debug_assert(constantLocation != GpuVariable_NULL);
    ::glProgramUniform4fv(mResourceHandle, constantLocation, 1, &floatVector4.x);
    glCheckError();
}

void GpuProgram::SetUniformParam(GpuVariableLocation constantLocation, const glm::mat3& floatMatrix3)
{
    debug_assert(constantLocation != GpuVariable_NULL);
    ::glProgramUniformMatrix3fv(mResourceHandle, constantLocation, 1, GL_FALSE, &floatMatrix3[0][0]);
    glCheckError();
}

void GpuProgram::SetUniformParam(GpuVariableLocation constantLocation, const glm::mat4& floatMatrix4)
{
    debug_assert(constantLocation != GpuVariable_NULL);
    ::glProgramUniformMatrix4fv(mResourceHandle, constantLocation, 1, GL_FALSE, &floatMatrix4[0][0]);
    glCheckError();
}

bool GpuProgram::CompileSourceCode(const char* shaderSource)
{
    // set unbound
    if (this == mGraphicsContext.mCurrentProgram)
    {
        mGraphicsContext.mCurrentProgram = nullptr;
    }

    bool isSuccessed = false;
    {
        // create temporary program
        GLuint programHandleGL = ::glCreateProgram();
        glCheckError();

        isSuccessed = CompileSourceCode(programHandleGL, shaderSource);
        if (!isSuccessed)
        {
            // destroy temporary program
            ::glDeleteProgram(programHandleGL);
            glCheckError();
        }
        else
        {
            FreeProgram();
            mResourceHandle = programHandleGL;
        }
    }

    if (!isSuccessed)
        return false;

    // clear old program data
    mInputLayout.mEnabledAttributes = 0;

    for (GpuVariableLocation& location: mAttributes) { location = GpuVariable_NULL; }
    for (GpuVariableLocation& location: mSamplers) { location = GpuVariable_NULL; }

    // query samplers
    for (int isampler = 0; isampler < eTextureUnit_COUNT; ++isampler)
    {
        GLint ilocation = ::glGetUniformLocation(mResourceHandle, cxx::enum_to_string((eTextureUnit) isampler));
        glCheckError();
        if (ilocation != GpuVariable_NULL)
        {
            mSamplers[isampler] = ilocation;
            // bind sampler to default slot
            ::glProgramUniform1i(mResourceHandle, ilocation, isampler);
            glCheckError();
        }
    }
    return true;
}

bool GpuProgram::CompileSourceCode(GpuProgramHandle targetHandle, const char* programSrc)
{
    // source strings
    const char* vertSource[] = { gGLSL_version_string, gGLSL_vertex_shader_string, programSrc };
    const char* fragSource[] = { gGLSL_version_string, gGLSL_fragment_shader_string, programSrc };

    // compile shaders
    GpuShader vertexShader (GL_VERTEX_SHADER);
    if (!vertexShader.Compile(vertSource, 3))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Vertex shader compilation failed");
        return false;
    }

    GpuShader fragmentShader (GL_FRAGMENT_SHADER);
    if (!fragmentShader.Compile(fragSource, 3))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Fragment shader compilation failed");
        return false;
    }

    // linking program
    GLint length;
    GLint linkResultGL;

    ::glAttachShader(targetHandle, vertexShader.mHandle);
    glCheckError();

    ::glAttachShader(targetHandle, fragmentShader.mHandle);
    glCheckError();

    ::glLinkProgram(targetHandle);
    glCheckError();

    ::glGetProgramiv(targetHandle, GL_LINK_STATUS, &linkResultGL);
    glCheckError();

    ::glGetProgramiv(targetHandle, GL_INFO_LOG_LENGTH, &length);
    glCheckError();

    if (length > 1) 
    {
        ::glGetProgramInfoLog(targetHandle, MaxShaderInfoLength, nullptr, ShaderMessageBuffer);
        glCheckError();

        gConsole.LogMessage(eLogMessage_Warning, "Program linkage message: '%s'", ShaderMessageBuffer);
    }

    if (linkResultGL == GL_FALSE)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Render program linkage error");
        return false;
    }
    return true;
}

void GpuProgram::FreeProgram()
{
    SetUnbound();
    if (mResourceHandle == GpuProgramHandle_NULL)
        return;

    ::glDeleteProgram(mResourceHandle);
    glCheckError();

    mResourceHandle = GpuProgramHandle_NULL;
}

GpuVariableLocation GpuProgram::QueryUniformLocation(const char* constantName) const
{
    GpuVariableLocation outLocation = ::glGetUniformLocation(mResourceHandle, constantName);
    glCheckError();

    return outLocation;
}

GpuVariableLocation GpuProgram::QueryAttributeLocation(const char* attributeName) const
{
    GpuVariableLocation outLocation = ::glGetAttribLocation(mResourceHandle, attributeName);
    glCheckError();

    return outLocation;
}

void GpuProgram::SetUnbound()
{
    if (this == mGraphicsContext.mCurrentProgram)
    {
        mGraphicsContext.mCurrentProgram = nullptr;
    }
}
