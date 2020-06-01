#pragma once

#include "GraphicsDefs.h"

// shader program
class RenderProgram: public cxx::noncopyable
{
public:
    // readonly
    std::string mProgramName;
    
    GpuProgram* mGpuProgram = nullptr;

public:
    // @param srcFileName: File name of shader source
    RenderProgram(const std::string& srcFilePath);
    virtual ~RenderProgram();

    // load shader program from source file, compile and create gpu object
    // @returns false on error
    bool LoadProgram();
    void FreeProgram();

    // try to reload from source file but not destroy current gpu object on fail
    // @returns false on error
    bool ReloadProgram();

    // test whether program is initialized and currently active
    bool IsProgramLoaded() const;
    bool IsProgramActive() const;

    // activate or deactivate render program
    void ActivateProgram();
    void DeactivateProgram();

    // set common constants
    void SetViewProjectionMatrix(const glm::mat4& viewProjectionMatrix);
    void SetModelMatrix(const glm::mat4& modelMatrix);
    void SetMaterialColorMode(eMaterialColorMode colorMode);

    virtual void HandleScreenResolutionChanged();

protected:
    // overridables
    virtual void HandleProgramLoad();
    virtual void HandleProgramFree();

protected:
    void SetupCommonConstants();
    void ClearCommonConstants();

protected:
    // common constants
    GpuVariableLocation mUniformID_view_projection_matrix = GpuVariable_NULL;
    GpuVariableLocation mUniformID_model_matrix = GpuVariable_NULL;
    GpuVariableLocation mUniformID_material_color_mode = GpuVariable_NULL;
};