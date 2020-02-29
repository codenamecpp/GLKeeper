#pragma once

#include "GraphicsDefs.h"

// defines hardware render program object
class GpuProgram: public cxx::noncopyable
{
    friend class GraphicsDevice;

public:
    GpuProgram(GraphicsDeviceContext& graphicsContext);
    ~GpuProgram();

    // create render program from shader source code
    // @param shaderSource: Source code
    bool CompileSourceCode(const char* shaderSource);

    // test whether render program is currently activated
    bool IsProgramBound() const;

    // test whether render program is compiled and ready
    bool IsProgramCompiled() const;

    // constant setters
    // @param constantLocation: Constant location
    void SetUniformParam(GpuVariableLocation constantLocation, float param0);
    void SetUniformParam(GpuVariableLocation constantLocation, float param0, float param1);
    void SetUniformParam(GpuVariableLocation constantLocation, float param0, float param1, float param2);
    void SetUniformParam(GpuVariableLocation constantLocation, int param0);
    void SetUniformParam(GpuVariableLocation constantLocation, const glm::vec2& floatVector2);
    void SetUniformParam(GpuVariableLocation constantLocation, const glm::vec3& floatVector3);
    void SetUniformParam(GpuVariableLocation constantLocation, const glm::vec4& floatVector4);
    void SetUniformParam(GpuVariableLocation constantLocation, const glm::mat3& floatMatrix3);
    void SetUniformParam(GpuVariableLocation constantLocation, const glm::mat4& floatMatrix4);

    // custom constants support
    // @param constantName: Uniform name
    // @returns -1 on uniform not exists
    GpuVariableLocation QueryUniformLocation(const char* constantName) const;

private:
    // implementation details
    bool CompileSourceCode(GpuProgramHandle targetHandle, const char* programSrc);
    void SetUnbound();

private:
    GpuProgramHandle mResourceHandle;
    GpuVariableLocation mAttributes[eVertexAttribute_MAX];
    GpuVariableLocation mSamplers[eTextureUnit_COUNT];
    RenderProgramInputLayout mInputLayout;
    GraphicsDeviceContext& mGraphicsContext;
};