#pragma once

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

    // free hardware program object
    void FreeProgram();

    // test whether render program is currently activated
    bool IsProgramBound() const;

    // test whether render program is compiled and ready
    bool IsProgramCompiled() const;

    // test whether vertex attributes was set
    bool IsProgramConfigured() const;

    // configure input layout
    bool BindAttribute(eVertexAttribute attributeIdentifier, const char* attributeName);

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

    // @param constantName: Uniform name
    GpuVariableLocation QueryUniformLocation(const char* constantName) const;

    // @param attributeName: Vertex attribute name
    GpuVariableLocation QueryAttributeLocation(const char* attributeName) const;

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