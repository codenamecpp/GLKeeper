#include "pch.h"
#include "RenderProgram.h"
#include "GpuProgram.h"
#include "GraphicsDevice.h"
#include "FileSystem.h"
#include "Console.h"

RenderProgram::RenderProgram(const std::string& srcFilePath)
    : mProgramSrcPath(srcFilePath)
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

    return ReloadProgram();
}

void RenderProgram::FreeProgram()
{
    if (IsProgramLoaded())
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
    if (!gFileSystem.ReadTextFile(mProgramSrcPath, shaderSourceCode))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read shader source from %s", mProgramSrcPath.c_str());
        return false;
    }

    bool isCompiled = mGpuProgram->CompileSourceCode(shaderSourceCode.c_str());
    if (isCompiled)
    {
        OnProgramLoad();
        gConsole.LogMessage(eLogMessage_Info, "Render program loaded %s", mProgramSrcPath.c_str());
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Info, "Cannot load render program %s", mProgramSrcPath.c_str());
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
    debug_assert(isInited);
    if (!isInited || mGpuProgram->IsProgramBound()) // program should be not active
        return;

    gGraphicsDevice.BindRenderProgram(mGpuProgram);
    OnProgramActivated();
}

void RenderProgram::DeactivateProgram()
{
    bool isInited = IsProgramLoaded();
    debug_assert(isInited);
    if (!isInited || !mGpuProgram->IsProgramBound()) // program should be active
        return;

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