#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GpuProgram.h"

//////////////////////////////////////////////////////////////////////////

class SurfaceMaterial;

//////////////////////////////////////////////////////////////////////////

class ShaderProgram : public cxx::noncopyable
{
public:
    ShaderProgram(const std::string& programName);
    virtual ~ShaderProgram();

    const std::string& GetProgramName() const { return mProgramName; }

    // loads program data from file; does not create render data automatically
    // will initialize default program if loading fails
    void Load();

    // init fallback program
    void InitDefault();

    // uploads system bits to backend; the program data must be inited beforehand
    // will init fallback render data on failure
    void InitRenderData();

    // reset system bits and destroy render data 
    void Purge();

    // set render program active
    void BindProgram();

    // returns true if either program is loaded from file or it is the default one
    inline bool IsInited() const { return mIsLoadedFromFile || mIsDefaultProgram; }
    inline bool IsLoadedFromFile() const { return mIsLoadedFromFile; }
    inline bool IsDefaultProgram() const { return mIsDefaultProgram; }
    inline bool IsRenderDataInited() const { return mIsRenderDataInited; }

    // set common uniforms
    void SetViewProjectionMatrix(const glm::mat4& sourceMatrix);
    void SetModelMatrix(const glm::mat4& sourceMatrix);
    void SetViewMatrix(const glm::mat4& sourceMatrix);
    void SetModelViewMatrix(const glm::mat4& sourceMatrix);
    void SetProjectionMatrix(const glm::mat4& sourceMatrix);
    void SetModelViewProjectionMatrix(const glm::mat4& sourceMatrix);
    void SetNormalMatrix(const glm::mat3& sourceMatrix);
    void SetCameraPosition(const glm::vec3& cameraPosition);

    // public overridables
    virtual void SetMaterialUniforms(const SurfaceMaterial& material);

    inline GpuProgram* GetGpuProgramPtr() const
    {
        return mGpuProgramResource.get();
    }

protected:
    // internal overridables
    virtual void HandleRenderDataInit();
    virtual void HandleRenderDataPurge();

private:
    bool LoadProgramFromFile();
    void GenerateCompleteShadersSource(std::string& vertShader, std::string& fragShader) const;
    void GenerateFallbackShadersSource(std::string& vertShader, std::string& fragShader) const;

protected:

    // common transformation uniforms
    GpuVariableLocation mGpuUniform_ViewProjMatrix = GpuLocation_Null;
    GpuVariableLocation mGpuUniform_ModelMatrix = GpuLocation_Null;
    GpuVariableLocation mGpuUniform_ViewMatrix = GpuLocation_Null;
    GpuVariableLocation mGpuUniform_ModelViewMatrix = GpuLocation_Null;
    GpuVariableLocation mGpuUniform_ProjectionMatrix = GpuLocation_Null;
    GpuVariableLocation mGpuUniform_MvpMatrix = GpuLocation_Null;
    GpuVariableLocation mGpuUniform_NormalMatrix = GpuLocation_Null;
    GpuVariableLocation mGpuUniform_CameraPosition = GpuLocation_Null;
    // common material uniforms
    GpuVariableLocation mGpuUniform_MaterialOpacity = GpuLocation_Null;
    GpuVariableLocation mGpuUniform_MaterialBaseColor = GpuLocation_Null;

private:
    std::string mProgramName;

    std::vector<std::string> mVertShaderSrcCode;
    std::vector<std::string> mFragShaderSrcCode;

    std::unique_ptr<GpuProgram> mGpuProgramResource;

    // status
    bool mIsLoadedFromFile = false;
    bool mIsDefaultProgram = false;
    bool mIsRenderDataInited = false;
};

//////////////////////////////////////////////////////////////////////////

class ShaderProgram_DebugDraw: public ShaderProgram
{
public:
    ShaderProgram_DebugDraw(const std::string& programName);
protected:
    // overrride ShaderProgram
    void HandleRenderDataInit() override;
    void HandleRenderDataPurge() override;
};

//////////////////////////////////////////////////////////////////////////

class ShaderProgram_Terrain: public ShaderProgram
{
public:
    ShaderProgram_Terrain(const std::string& programName);
protected:
    // overrride ShaderProgram
    void HandleRenderDataInit() override;
    void HandleRenderDataPurge() override;
};

//////////////////////////////////////////////////////////////////////////

class ShaderProgram_WaterLava: public ShaderProgram
{
public:
    ShaderProgram_WaterLava(const std::string& programName);
    void SetWaterLavaParams(float waveTime, float waveWidth, float waveHeight, float waterLine);
protected:
    // overrride ShaderProgram
    void HandleRenderDataInit() override;
    void HandleRenderDataPurge() override;
protected:
    GpuVariableLocation mGpuUniform_WaveTime = GpuLocation_Null;
    GpuVariableLocation mGpuUniform_WaveWidth = GpuLocation_Null;
    GpuVariableLocation mGpuUniform_WaveHeight = GpuLocation_Null;
    GpuVariableLocation mGpuUniform_WaterLine = GpuLocation_Null;
};

//////////////////////////////////////////////////////////////////////////

class ShaderProgram_BlendFrames: public ShaderProgram
{
public:
    ShaderProgram_BlendFrames(const std::string& programName);
    void SetMixFrames(float mixFrames);
protected:
    // overrride ShaderProgram
    void HandleRenderDataInit() override;
    void HandleRenderDataPurge() override;
protected:
    GpuVariableLocation mGpuUniform_MixFrames = GpuLocation_Null;
};

//////////////////////////////////////////////////////////////////////////

class ShaderProgram_UI: public ShaderProgram
{
public:
    ShaderProgram_UI(const std::string& programName);
    void SetTexturesEnabled(bool isEnabled);
    void SetTextureAlpha(bool isEnabled);
protected:
    // overrride ShaderProgram
    void HandleRenderDataInit() override;
    void HandleRenderDataPurge() override;
protected:
    GpuVariableLocation mGpuUniform_EnableTextures = GpuLocation_Null;
    GpuVariableLocation mGpuUniform_TextureA8 = GpuLocation_Null;
};

//////////////////////////////////////////////////////////////////////////

class ShaderProgram_StaticMesh: public ShaderProgram
{
public:
    ShaderProgram_StaticMesh(const std::string& programName);
protected:
    // overrride ShaderProgram
    void HandleRenderDataInit() override;
    void HandleRenderDataPurge() override;
};

//////////////////////////////////////////////////////////////////////////