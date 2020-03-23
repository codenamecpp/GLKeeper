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
        OnProgramFree();

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
        OnProgramLoad();
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
    OnProgramActivated();
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
    OnProgramDeactivated();
}

void RenderProgram::OnProgramActivated()
{
    // do nothing
}

void RenderProgram::OnProgramDeactivated()
{
    // do nothing
}

void RenderProgram::OnProgramLoad()
{
    // do nothing
}

void RenderProgram::OnProgramFree()
{
    // do nothing
}