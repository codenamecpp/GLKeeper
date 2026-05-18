#include "stdafx.h"
#include "ShaderProgram.h"
#include "SurfaceMaterial.h"

ShaderProgram::ShaderProgram(const std::string& programName)
    : mProgramName(programName)
{
}

ShaderProgram::~ShaderProgram()
{

}

void ShaderProgram::Load()
{
    Purge();

    if (LoadProgramFromFile())
    {
        mIsLoadedFromFile = true;
    }
    else
    {
        InitDefault();
    }
}

void ShaderProgram::InitDefault()
{
    Purge();

    mIsDefaultProgram = true;
}

void ShaderProgram::Purge()
{
    if (IsRenderDataInited())
    {
        HandleRenderDataPurge();
    }

    mIsDefaultProgram = false;
    mIsLoadedFromFile = false;
    mIsRenderDataInited = false;

    mVertShaderSrcCode.clear();
    mFragShaderSrcCode.clear();

    mGpuProgramResource.reset();
}

void ShaderProgram::BindProgram()
{
    InitRenderData();
    gRenderDevice.BindShaderProgram(GetGpuProgramPtr());
}

void ShaderProgram::SetViewProjectionMatrix(const glm::mat4& sourceMatrix)
{
    bool isSuccess = (mGpuUniform_ViewProjMatrix != GpuLocation_Null);
    if (isSuccess)
    {
        cxx_assert(mGpuProgramResource);
        mGpuProgramResource->SetUniform(mGpuUniform_ViewProjMatrix, sourceMatrix);
    }
    cxx_assert(isSuccess);
}

void ShaderProgram::SetModelMatrix(const glm::mat4& sourceMatrix)
{
    bool isSuccess = (mGpuUniform_ModelMatrix != GpuLocation_Null);
    if (isSuccess)
    {
        cxx_assert(mGpuProgramResource);
        mGpuProgramResource->SetUniform(mGpuUniform_ModelMatrix, sourceMatrix);
    }
    cxx_assert(isSuccess);
}

void ShaderProgram::SetViewMatrix(const glm::mat4& sourceMatrix)
{
    bool isSuccess = (mGpuUniform_ViewMatrix != GpuLocation_Null);
    if (isSuccess)
    {
        cxx_assert(mGpuProgramResource);
        mGpuProgramResource->SetUniform(mGpuUniform_ViewMatrix, sourceMatrix);
    }
    cxx_assert(isSuccess);
}

void ShaderProgram::SetModelViewMatrix(const glm::mat4& sourceMatrix)
{
    bool isSuccess = (mGpuUniform_ModelViewMatrix != GpuLocation_Null);
    if (isSuccess)
    {
        cxx_assert(mGpuProgramResource);
        mGpuProgramResource->SetUniform(mGpuUniform_ModelViewMatrix, sourceMatrix);
    }
    cxx_assert(isSuccess);
}

void ShaderProgram::SetProjectionMatrix(const glm::mat4& sourceMatrix)
{
    bool isSuccess = (mGpuUniform_ProjectionMatrix != GpuLocation_Null);
    if (isSuccess)
    {
        cxx_assert(mGpuProgramResource);
        mGpuProgramResource->SetUniform(mGpuUniform_ProjectionMatrix, sourceMatrix);
    }
    cxx_assert(isSuccess);
}

void ShaderProgram::SetModelViewProjectionMatrix(const glm::mat4& sourceMatrix)
{
    bool isSuccess = (mGpuUniform_MvpMatrix != GpuLocation_Null);
    if (isSuccess)
    {
        cxx_assert(mGpuProgramResource);
        mGpuProgramResource->SetUniform(mGpuUniform_MvpMatrix, sourceMatrix);
    }
    cxx_assert(isSuccess);
}

void ShaderProgram::SetNormalMatrix(const glm::mat3& sourceMatrix)
{
    bool isSuccess = (mGpuUniform_NormalMatrix != GpuLocation_Null);
    if (isSuccess)
    {
        cxx_assert(mGpuProgramResource);
        mGpuProgramResource->SetUniform(mGpuUniform_NormalMatrix, sourceMatrix);
    }
    cxx_assert(isSuccess);
}

void ShaderProgram::SetCameraPosition(const glm::vec3& cameraPosition)
{
    bool isSuccess = (mGpuUniform_CameraPosition != GpuLocation_Null);
    if (isSuccess)
    {
        cxx_assert(mGpuProgramResource);
        mGpuProgramResource->SetUniform(mGpuUniform_CameraPosition, cameraPosition);
    }
    cxx_assert(isSuccess);
}

void ShaderProgram::SetMaterialUniforms(const SurfaceMaterial& material)
{
    if (!material.IsOpaque())
    {
        if (mGpuUniform_MaterialOpacity != GpuLocation_Null)
        {
            mGpuProgramResource->SetUniform(mGpuUniform_MaterialOpacity, material.mOpacity);
        }
    }

    if (mGpuUniform_MaterialBaseColor != GpuLocation_Null)
    {
        mGpuProgramResource->SetUniform(mGpuUniform_MaterialBaseColor, material.mBaseColor.ToFloats3());
    }
}

void ShaderProgram::InitRenderData()
{
    if (IsRenderDataInited())
        return;

    if (!IsInited())
    {
        cxx_assert(false);
        return;
    }

    mIsRenderDataInited = true;

    std::string vertShaderSrc;
    std::string fragShaderSrc;
    if (!IsDefaultProgram())
    {
        GenerateCompleteShadersSource(vertShaderSrc, fragShaderSrc);

        mGpuProgramResource = gRenderDevice.CreateShaderProgram(vertShaderSrc, fragShaderSrc);
        if (mGpuProgramResource)
        {
            HandleRenderDataInit();
            return;
        }

        cxx_assert(false);
    }

    gConsole.LogMessage(eLogLevel_Warning, "Cannot initialize shader program render data ('%s'), trying fallback", mProgramName.c_str());

    // fallback
    GenerateFallbackShadersSource(vertShaderSrc, fragShaderSrc);
    mGpuProgramResource = gRenderDevice.CreateShaderProgram(vertShaderSrc, fragShaderSrc);
    cxx_assert(mGpuProgramResource);
    if (mGpuProgramResource)
    {
        HandleRenderDataInit();
    }
}

void ShaderProgram::HandleRenderDataInit()
{
    cxx_assert(mGpuProgramResource);
    if (mGpuProgramResource)
    {
        // view projection matrix
        mGpuUniform_ViewProjMatrix = mGpuProgramResource->QueryUniformLocation("u_view_proj");
        // model matrix
        mGpuUniform_ModelMatrix = mGpuProgramResource->QueryUniformLocation("u_model_matrix");
        // view matrix
        mGpuUniform_ViewMatrix = mGpuProgramResource->QueryUniformLocation("u_view_matrix");
        // projection matrix
        mGpuUniform_ProjectionMatrix = mGpuProgramResource->QueryUniformLocation("u_proj_matrix");
        // model view matrix
        mGpuUniform_ModelViewMatrix = mGpuProgramResource->QueryUniformLocation("u_model_view");
        // model view projection matrix
        mGpuUniform_MvpMatrix = mGpuProgramResource->QueryUniformLocation("u_MVP");
        // normal matrix
        mGpuUniform_NormalMatrix = mGpuProgramResource->QueryUniformLocation("u_norm_matrix");
        // camera position
        mGpuUniform_CameraPosition = mGpuProgramResource->QueryUniformLocation("u_camera_pos");

        // common material uniforms
        mGpuUniform_MaterialOpacity = mGpuProgramResource->QueryUniformLocation("u_material_opacity");
        mGpuUniform_MaterialBaseColor = mGpuProgramResource->QueryUniformLocation("u_material_base_color");
    }
}

void ShaderProgram::HandleRenderDataPurge()
{
    mGpuUniform_ViewProjMatrix = GpuLocation_Null;
    mGpuUniform_ModelMatrix = GpuLocation_Null;
    mGpuUniform_ViewMatrix = GpuLocation_Null;
    mGpuUniform_ProjectionMatrix = GpuLocation_Null;
    mGpuUniform_ModelViewMatrix = GpuLocation_Null;
    mGpuUniform_MvpMatrix = GpuLocation_Null;
    mGpuUniform_NormalMatrix = GpuLocation_Null;
    mGpuUniform_CameraPosition = GpuLocation_Null;
    mGpuUniform_MaterialOpacity = GpuLocation_Null;
    mGpuUniform_MaterialBaseColor = GpuLocation_Null;
}

bool ShaderProgram::LoadProgramFromFile()
{
    // helper
    auto trimStrings = [](std::vector<std::string>& container)
    {
        for (std::string& roller: container)
        {
            cxx::trim_right(roller);
        }
    };

    const std::string& programName = mProgramName;

    if (programName.empty())
    {
        cxx_assert(false);
        return false;
    }

    // vertex shader
    std::string vertShaderName = programName + ".vert";
    std::string vertShaderFilePath;
    if (!gFiles.LocateShader(vertShaderName, vertShaderFilePath))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Vertex shader missing '%s'", vertShaderName.c_str());
        return false;
    }

    if (!FSReadTextLinesFromFile(vertShaderFilePath, mVertShaderSrcCode))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot read vertex shader '%s'", vertShaderName.c_str());
        return false;
    }

    // fragment shader
    std::string fragShaderName = programName + ".frag";
    std::string fragShaderFilePath;
    if (!gFiles.LocateShader(fragShaderName, fragShaderFilePath))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Fragment shader missing '%s'", fragShaderName.c_str());
        return false;
    }

    if (!FSReadTextLinesFromFile(fragShaderFilePath, mFragShaderSrcCode))
    {
        gConsole.LogMessage(eLogLevel_Warning, "Cannot read fragment shader '%s'", fragShaderName.c_str());
        return false;
    }

    trimStrings(mVertShaderSrcCode);
    trimStrings(mFragShaderSrcCode);
    return true;
}

void ShaderProgram::GenerateCompleteShadersSource(std::string& vertShader, std::string& fragShader) const
{
    fragShader.clear();
    vertShader.clear();

    int fragShaderSrcLength = 0;
    int vertShaderSrcLength = 0;

    for (const std::string& roller: mFragShaderSrcCode) { fragShaderSrcLength += roller.length() + 1; } // +1 extra new line character
    for (const std::string& roller: mVertShaderSrcCode) { vertShaderSrcLength += roller.length() + 1; } // +1 extra new line character

    fragShader.reserve(fragShaderSrcLength);
    vertShader.reserve(vertShaderSrcLength);

    for (const std::string& roller: mFragShaderSrcCode) { fragShader += roller; fragShader.push_back('\n'); } // +1 extra new line character
    for (const std::string& roller: mVertShaderSrcCode) { vertShader += roller; vertShader.push_back('\n'); } // +1 extra new line character
}

void ShaderProgram::GenerateFallbackShadersSource(std::string& vertShader, std::string& fragShader) const
{
    fragShader = R"(
        in vec4 Color;
        out vec4 FinalColor;
        void main() 
        {
            FinalColor = Color;
        }
    )";

    vertShader = R"(
        uniform mat4 u_view_proj;
        in vec3 in_position0;
        in vec4 in_color0;
        out vec4 Color;
        void main() 
        {
	        Color = in_color0;
            gl_Position = u_view_proj * vec4(in_position0, 1.0f);
        }
    )";
}

//////////////////////////////////////////////////////////////////////////

ShaderProgram_DebugDraw::ShaderProgram_DebugDraw(const std::string& programName)
    : ShaderProgram(programName)
{
}

void ShaderProgram_DebugDraw::HandleRenderDataInit()
{
    ShaderProgram::HandleRenderDataInit();
}

void ShaderProgram_DebugDraw::HandleRenderDataPurge()
{
    ShaderProgram::HandleRenderDataPurge();
}

//////////////////////////////////////////////////////////////////////////

ShaderProgram_Terrain::ShaderProgram_Terrain(const std::string& programName)
    : ShaderProgram(programName)
{
}

void ShaderProgram_Terrain::HandleRenderDataInit()
{
    ShaderProgram::HandleRenderDataInit();
}

void ShaderProgram_Terrain::HandleRenderDataPurge()
{
    ShaderProgram::HandleRenderDataPurge();
}

//////////////////////////////////////////////////////////////////////////

ShaderProgram_WaterLava::ShaderProgram_WaterLava(const std::string& programName)
    : ShaderProgram(programName)
{
}

void ShaderProgram_WaterLava::SetWaterLavaParams(float waveTime, float waveWidth, float waveHeight, float waterLine)
{
    GpuProgram* gpuProgramResource = GetGpuProgramPtr();

    if (gpuProgramResource)
    {
        gpuProgramResource->SetUniform(mGpuUniform_WaveTime, waveTime);
        gpuProgramResource->SetUniform(mGpuUniform_WaveWidth, waveWidth);
        gpuProgramResource->SetUniform(mGpuUniform_WaveHeight, waveHeight);
        gpuProgramResource->SetUniform(mGpuUniform_WaterLine, waterLine);
    }
}

void ShaderProgram_WaterLava::HandleRenderDataInit()
{
    ShaderProgram::HandleRenderDataInit();

    GpuProgram* gpuProgramResource = GetGpuProgramPtr();
    cxx_assert(gpuProgramResource);

    mGpuUniform_WaveTime = gpuProgramResource->QueryUniformLocation("u_waveTime");
    mGpuUniform_WaveWidth = gpuProgramResource->QueryUniformLocation("u_waveWidth");
    mGpuUniform_WaveHeight = gpuProgramResource->QueryUniformLocation("u_waveHeight");
    mGpuUniform_WaterLine = gpuProgramResource->QueryUniformLocation("u_waterLine");
}

void ShaderProgram_WaterLava::HandleRenderDataPurge()
{
    ShaderProgram::HandleRenderDataPurge();

    mGpuUniform_WaveTime = GpuLocation_Null;
    mGpuUniform_WaveWidth = GpuLocation_Null;
    mGpuUniform_WaveHeight = GpuLocation_Null;
    mGpuUniform_WaterLine = GpuLocation_Null;
}

//////////////////////////////////////////////////////////////////////////

ShaderProgram_BlendFrames::ShaderProgram_BlendFrames(const std::string& programName)
    : ShaderProgram(programName)
{
}

void ShaderProgram_BlendFrames::SetMixFrames(float mixFrames)
{
    GpuProgram* gpuProgramResource = GetGpuProgramPtr();
    if (gpuProgramResource)
    {
        gpuProgramResource->SetUniform(mGpuUniform_MixFrames, mixFrames);
    }
}

void ShaderProgram_BlendFrames::HandleRenderDataInit()
{
    ShaderProgram::HandleRenderDataInit();

    GpuProgram* gpuProgramResource = GetGpuProgramPtr();
    cxx_assert(gpuProgramResource);

    mGpuUniform_MixFrames = gpuProgramResource->QueryUniformLocation("u_mixFrames");
}

void ShaderProgram_BlendFrames::HandleRenderDataPurge()
{
    ShaderProgram::HandleRenderDataPurge();

    mGpuUniform_MixFrames = GpuLocation_Null;
}

//////////////////////////////////////////////////////////////////////////

ShaderProgram_UI::ShaderProgram_UI(const std::string& programName)
    : ShaderProgram(programName)
{
}

void ShaderProgram_UI::SetTexturesEnabled(bool isEnabled)
{
    GpuProgram* gpuProgramResource = GetGpuProgramPtr();
    if (gpuProgramResource)
    {
        gpuProgramResource->SetUniform(mGpuUniform_EnableTextures, isEnabled ? 1 : 0);
    }
}

void ShaderProgram_UI::SetTextureAlpha(bool isEnabled)
{
    GpuProgram* gpuProgramResource = GetGpuProgramPtr();
    if (gpuProgramResource)
    {
        gpuProgramResource->SetUniform(mGpuUniform_TextureA8, isEnabled ? 1 : 0);
    }
}

void ShaderProgram_UI::HandleRenderDataInit()
{
    ShaderProgram::HandleRenderDataInit();

    GpuProgram* gpuProgramResource = GetGpuProgramPtr();
    cxx_assert(gpuProgramResource);

    mGpuUniform_EnableTextures = gpuProgramResource->QueryUniformLocation("u_TextureEnabled");
    mGpuUniform_TextureA8 = gpuProgramResource->QueryUniformLocation("u_TextureA8");
}

void ShaderProgram_UI::HandleRenderDataPurge()
{
    ShaderProgram::HandleRenderDataPurge();

    mGpuUniform_EnableTextures = GpuLocation_Null;
    mGpuUniform_TextureA8 = GpuLocation_Null;
}

//////////////////////////////////////////////////////////////////////////

ShaderProgram_StaticMesh::ShaderProgram_StaticMesh(const std::string& programName)
    : ShaderProgram(programName)
{
}

void ShaderProgram_StaticMesh::HandleRenderDataInit()
{
    ShaderProgram::HandleRenderDataInit();
}

void ShaderProgram_StaticMesh::HandleRenderDataPurge()
{
    ShaderProgram::HandleRenderDataPurge();
}
