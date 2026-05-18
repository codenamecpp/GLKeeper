#pragma once

#include "GraphicsDefs.h"

// Defines hardware render program object
class GpuProgram: public cxx::noncopyable
{
    friend class RenderDevice;

public:
    GpuProgram();
    ~GpuProgram();

    // Create render program from shader source code
    // @param shaderSource: Source code
    bool CompileShader(const std::string& vertShaderSrc, const std::string& fragShaderSrc);

    // Test whether render program is currently activated
    bool IsProgramBound() const;

    // Test whether render program is compiled and ready
    bool IsProgramCompiled() const;

    bool BindAttribute(eVertexAttribute attributeIdentifier, const char* attributeName);

    // constant setters
    void SetUniform(GpuVariableLocation uniformLocation, float param0);
    void SetUniform(GpuVariableLocation uniformLocation, float param0, float param1);
    void SetUniform(GpuVariableLocation uniformLocation, float param0, float param1, float param2);
    void SetUniform(GpuVariableLocation uniformLocation, int param0);
    void SetUniform(GpuVariableLocation uniformLocation, const glm::vec2& floatVector2);
    void SetUniform(GpuVariableLocation uniformLocation, const glm::vec3& floatVector3);
    void SetUniform(GpuVariableLocation uniformLocation, const glm::vec4& floatVector4);
    void SetUniform(GpuVariableLocation uniformLocation, const glm::mat3& floatMatrix3);
    void SetUniform(GpuVariableLocation uniformLocation, const glm::mat4& floatMatrix4);

    // @param constantName: Uniform name
    GpuVariableLocation QueryUniformLocation(const char* constantName) const;

    // @param attributeName: Vertex attribute name
    GpuVariableLocation QueryAttributeLocation(const char* attributeName) const;

private:
    bool CompileShader(GpuResourceHandle targetHandle, const char* vertShaderSrc, const char* fragShaderSrc);

private:
    // shared render device context data
    static GpuProgram* sCurrentProgram;

private:
    GpuResourceHandle mResourceHandle;
    GpuVariableLocation mAttributes[eVertexAttribute_MAX];
    GpuVariableLocation mSamplers[eTextureUnit_COUNT];
    ShaderProgramInputLayout mInputLayout;
};