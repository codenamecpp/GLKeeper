#include "stdafx.h"
#include "MeshAssetManager.h"

//////////////////////////////////////////////////////////////////////////

MeshAssetManager gMeshAssetManager;

//////////////////////////////////////////////////////////////////////////

void MeshAssetManager::Initialize()
{

}

void MeshAssetManager::Shutdown()
{
    mResourcesMap.clear();
}

MeshAsset* MeshAssetManager::LoadMesh(const std::string& name, const LoadParams& params /*= {}*/)
{
    if (name.empty())
    {
        cxx_assert(false);
        return nullptr;
    }

    MeshAsset* meshInstance = FindMesh(name);
    cxx_assert(meshInstance == nullptr);
    if (meshInstance)
    {
        gConsole.LogMessage(eLogLevel_Debug, "Mesh '%s' is already loaded", name.c_str());
    }
    else
    {
        std::unique_ptr<MeshAsset> newMeshInstance = std::make_unique<MeshAsset>(name);
        newMeshInstance->Load();

        bool isSuccess = newMeshInstance->IsLoadedFromFile() || (params.mInitDefaultOnFail && newMeshInstance->IsDefaultMesh());
        if (isSuccess)
        {
            meshInstance = newMeshInstance.get();
            mResourcesMap[name] = std::move(newMeshInstance);
        }
    }
    return meshInstance;
}

MeshAsset* MeshAssetManager::FindMesh(const std::string& name) const
{
    auto mapIt = mResourcesMap.find(name);
    if (mapIt != mResourcesMap.end())
    {
        return mapIt->second.get();
    }
    return nullptr;
}

MeshAsset* MeshAssetManager::GetMesh(const std::string& name, const LoadParams& params)
{
    MeshAsset* meshInstance = FindMesh(name);
    if (meshInstance == nullptr)
    {
        meshInstance = LoadMesh(name, params);
    }
    cxx_assert(meshInstance);
    return meshInstance;
}
