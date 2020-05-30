#include "pch.h"
#include "RenderProgram.h"
#include "GpuProgram.h"
#include "GraphicsDevice.h"
#include "FileSystem.h"
#include "Console.h"
#include "RenderManager.h"

RenderProgram::RenderProgram(const std::string& srcFilePath)
    : mProgramName(srcFilePath)
{
}

RenderProgram::~RenderProgram()
{
    debug_assert(mGpuProgram == nullptr);
}

bool RenderProgram::LoadProgram()
{
    if (IsProgramLoaded())
        return true;

    bool isSuccess = ReloadProgram();
    if (isSuccess)
    {
        gRenderManager.HandleRenderProgramLoad(this);
    }
    return isSuccess;
}

void RenderProgram::FreeProgram()
{
    if (IsProgramLoaded())
    {
        gRenderManager.HandleRenderProgramFree(this);
    }

    if (mGpuProgram)
    {
        DeactivateProgram();
        gGraphicsDevice.DestroyProgram(mGpuProgram);
        HandleProgramFree();
        ClearCommonConstants();

        mGpuProgram = nullptr;
    }
}

bool RenderProgram::ReloadProgram()
{
    if (mGpuProgram == nullptr)
    {
        mGpuProgram = gGraphicsDevice.CreateRenderProgram();
        if (mGpuProgram == nullptr)
        {
            gConsole.LogMessage(eLogMessage_Warning, "Cannot create render program object");
            return false;
        }
    }

    // load source code
    std::string shaderSourceCode;
    if (!gFileSystem.ReadTextFile(mProgramName, shaderSourceCode))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read shader source from %s", mProgramName.c_str());
        return false;
    }

    bool isCompiled = mGpuProgram->CompileSourceCode(shaderSourceCode.c_str());
    if (isCompiled)
    {
        HandleProgramLoad();
        SetupCommonConstants();
        gConsole.LogMessage(eLogMessage_Debug, "Render program loaded %s", mProgramName.c_str());
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load render program %s", mProgramName.c_str());
    }
    return isCompiled;
}

bool RenderProgram::IsProgramLoaded() const
{
    return mGpuProgram && mGpuProgram->IsProgramCompiled();
}

bool RenderProgram::IsProgramActive() const
{
    return mGpuProgram && mGpuProgram->IsProgramBound();
}

void RenderProgram::ActivateProgram()
{
    bool isInited = IsProgramLoaded();
    if (!isInited || mGpuProgram->IsProgramBound()) // program should be not active
        return;

    // set active render program
    debug_assert(gRenderManager.mActiveRenderProgram != this);
    if (gRenderManager.mActiveRenderProgram)
    {
        gRenderManager.mActiveRenderProgram->DeactivateProgram();
    }
    gRenderManager.mActiveRenderProgram = this;

    gGraphicsDevice.BindRenderProgram(mGpuProgram);
}

void RenderProgram::DeactivateProgram()
{
    bool isInited = IsProgramLoaded();
    if (!isInited || !mGpuProgram->IsProgramBound()) // program should be active
        return;

    // clear active render program
    debug_assert(gRenderManager.mActiveRenderProgram == this);
    if (gRenderManager.mActiveRenderProgram == this)
    {
        gRenderManager.mActiveRenderProgram = nullptr;
    }

    gGraphicsDevice.BindRenderProgram(nullptr);
}

void RenderProgram::SetViewProjectionMatrix(const glm::mat4& viewProjectionMatrix)
{
    if (!IsProgramLoaded())
    {
        debug_assert(false);
        return;
    }

    mGpuProgram->SetUniformParam(mUniformID_view_projection_matrix, viewProjectionMatrix);
}

void RenderProgram::SetModelMatrix(const glm::mat4& modelMatrix)
{
    if (!IsProgramLoaded())
    {
        debug_assert(false);
        return;
    }

    mGpuProgram->SetUniformParam(mUniformID_model_matrix, modelMatrix);
}

void RenderProgram::HandleScreenResolutionChanged()
{
    // do nothing
}

void RenderProgram::HandleProgramLoad()
{
    // do nothing
}

void RenderProgram::HandleProgramFree()
{
    // do nothing
}

void RenderProgram::SetupCommonConstants()
{
    // viewprojectionmatrix
    if (mUniformID_view_projection_matrix == GpuVariable_NULL) 
    {
        mUniformID_view_projection_matrix = mGpuProgram->QueryUniformLocation("view_projection_matrix");
    }

    // modelmatrix
    if (mUniformID_model_matrix == GpuVariable_NULL)
    {
        mUniformID_model_matrix = mGpuProgram->QueryUniformLocation("model_matrix");
    }
}

void RenderProgram::ClearCommonConstants()
{
    mUniformID_view_projection_matrix = GpuVariable_NULL;
    mUniformID_model_matrix = GpuVariable_NULL;
}
