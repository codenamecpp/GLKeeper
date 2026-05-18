#include "stdafx.h"
#include "ShadersManager.h"

//////////////////////////////////////////////////////////////////////////

ShadersManager gShadersManager;

//////////////////////////////////////////////////////////////////////////

void ShadersManager::Initialize()
{

}

void ShadersManager::Shutdown()
{
    mResourcesMap.clear();
}

ShaderProgram* ShadersManager::LoadProgram(const std::string& name, const LoadParams& params)
{
    if (name.empty())
    {
        cxx_assert(false);
        return nullptr;
    }

    ShaderProgram* programInstance = FindProgram(name);
    cxx_assert(programInstance == nullptr);
    if (programInstance)
    {
        gConsole.LogMessage(eLogLevel_Debug, "Shader Program '%s' is already loaded", name.c_str());
    }
    else
    {
        std::unique_ptr<ShaderProgram> newInstance = params.mFactory ? params.mFactory->CreateInstance(name) : std::make_unique<ShaderProgram>(name);
        newInstance->Load();

        bool iSuccess = newInstance->IsLoadedFromFile() || (params.mInitDefaultOnFail && newInstance->IsDefaultProgram());
        if (iSuccess)
        {
            programInstance = newInstance.get();
            mResourcesMap[name] = std::move(newInstance);
        }
    }
    return programInstance;
}

ShaderProgram* ShadersManager::FindProgram(const std::string& name) const
{
    auto mapIt = mResourcesMap.find(name);
    if (mapIt != mResourcesMap.end())
    {
        return mapIt->second.get();
    }
    return nullptr;
}

ShaderProgram* ShadersManager::GetProgram(const std::string& name, const LoadParams& params)
{
    ShaderProgram* programInstance = FindProgram(name);
    if (programInstance == nullptr)
    {
        programInstance = LoadProgram(name, params);
    }
    cxx_assert(programInstance);
    return programInstance;
}
