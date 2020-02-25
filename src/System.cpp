#include "pch.h"
#include "System.h"
#include "Console.h"
#include "FileSystem.h"
#include "InputsManager.h"
#include "GraphicsDevice.h"
#include "EngineTexturesProvider.h"
#include "RenderManager.h"
#include "TimeManager.h"

System gSystem;

void System::Initialize(int argc, char *argv[])
{
    if (!gConsole.Initialize())
    {
        debug_assert(false);
    }

    gConsole.LogMessage(eLogMessage_Info, GAME_TITLE);
    gConsole.LogMessage(eLogMessage_Info, "System initialize");

    if (!gFileSystem.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize filesystem");
        Terminate();
    }

    if (!mStartupParams.ParseStartupParams(argc, argv))
    {
        debug_assert(false);
    }

    LoadSettings();

    if (!gFileSystem.SetupDungeonKeeperGamePaths())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize game paths");
        Terminate();
    }
    
    if (!gFileSystem.MountDungeonKeeperGameArchives())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot mount game resource archives");
        Terminate();
    }

    if (!gEngineTexturesProvider.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize engine textures provider");
        Terminate();
    }

    if (!gInputsManager.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize inputs manager");
        Terminate();
    }

    if (!gGraphicsDevice.Initialize(mSettings.mScreenSizex, mSettings.mScreenSizey, mSettings.mFullscreen, mSettings.mEnableVSync))
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize graphics device");
        Terminate();
    }

    gTimeManager.Initialize();

    if (!gRenderManager.Initialize())
    {
        gConsole.LogMessage(eLogMessage_Error, "Cannot initialize render manager");
        Terminate();
    }

    mStartSystemTime = ::glfwGetTime();
    debug_assert(mStartSystemTime > 0.0);

    mQuitRequested = false;
}

void System::Deinit()
{
    gConsole.LogMessage(eLogMessage_Info, "System shutdown");

    gRenderManager.Deinit();
    gTimeManager.Deinit();
    gGraphicsDevice.Deinit();
    gInputsManager.Deinit();
    gEngineTexturesProvider.Deinit();
    gFileSystem.Deinit();
    gConsole.Deinit();
}

void System::Execute()
{
    const double MaxFPS = 1000.0;
    const double MinFPS = 20.0;
    const double MaxFrameDelta = (1.0 / MinFPS);
    const double MinFrameDelta = (1.0 / MaxFPS);

    const long long MinFrameDeltaMilliseconds = static_cast<long long>(MinFrameDelta * 1000.0);

    // main loop
    double previousFrameTime = ::glfwGetTime();
    for (; !mQuitRequested; )
    {
        double currentFrameTime = ::glfwGetTime();
        double currentFrameDelta = currentFrameTime - previousFrameTime;

        if (currentFrameDelta > MaxFrameDelta)
        {
            currentFrameDelta = MaxFrameDelta;
        }

        if (currentFrameDelta < MinFrameDelta)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(MinFrameDeltaMilliseconds));
            currentFrameDelta = MinFrameDelta;

            currentFrameTime = ::glfwGetTime();
        }

        gTimeManager.UpdateFrame(currentFrameDelta);
        gGraphicsDevice.ProcessInputEvents();

        if (mQuitRequested)
            break;

        // todo process game logic

        gRenderManager.RenderFrame();
        previousFrameTime = currentFrameTime;
    }
}

void System::Terminate()
{
    Deinit(); // leave gracefully

    exit(EXIT_FAILURE);
}

void System::QuitRequest()
{
    mQuitRequested = true;
}

double System::GetSysTime() const
{
    double currentTime = ::glfwGetTime();
    return currentTime - mStartSystemTime;
}

bool System::SaveSettings()
{
    return true;
}

bool System::LoadSettings()
{
    mSettings.SetDefaults();

    return true;
}