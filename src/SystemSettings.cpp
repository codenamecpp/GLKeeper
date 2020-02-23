#include "pch.h"
#include "SystemSettings.h"
#include "Console.h"

const int DefaultScreenResolutionX = 1024;
const int DefaultScreenResolutionY = 768;

//////////////////////////////////////////////////////////////////////////

void SystemSettings::SetDefaults()
{
    mScreenSizex = DefaultScreenResolutionX;
    mScreenSizey = DefaultScreenResolutionY; 
    mFullscreen = false;
    mEnableVSync = false;
}

//////////////////////////////////////////////////////////////////////////

bool SystemStartupParams::ParseStartupParams(int argc, char *argv[])
{
    Clear();

    for (int iarg = 0; iarg < argc; )
    {
        if (cxx_stricmp(argv[iarg], "-config") == 0 && (argc > iarg + 1))
        {
            mCustomConfigFileName.assign(argv[iarg + 1]);

            iarg += 2;
            continue;
        }

        if (cxx_stricmp(argv[iarg], "-gamedir") == 0 && (argc > iarg + 1))
        {
            mDungeonKeeperGamePath.assign(argv[iarg + 1]);

            iarg += 2;
            continue;
        }

        ++iarg;
    }

    return true;
}

void SystemStartupParams::Clear()
{
    mCustomConfigFileName.clear();
    mDungeonKeeperGamePath.clear();
}
