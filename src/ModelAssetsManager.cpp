#include "pch.h"
#include "ModelAssetsManager.h"
#include "FileSystem.h"
#include "Console.h"
#include "ModelAsset.h"

ModelAssetsManager gModelsManager;

bool ModelAssetsManager::Initialize()
{
    return true;
}

void ModelAssetsManager::Deinit()
{
    for (auto& currRecord: mModelAssetsMap)
    {
        delete currRecord.second;
    }
    mModelAssetsMap.clear();
}

ModelAsset* ModelAssetsManager::FindModelAsset(const std::string& resourceName) const
{
    auto models_iterator = mModelAssetsMap.find(resourceName);
    if (models_iterator != mModelAssetsMap.end())
    {
        return models_iterator->second;
    }
    return nullptr;
}

ModelAsset* ModelAssetsManager::LoadModelAsset(const std::string& resourceName)
{
    ModelAsset* modelResource = FindModelAsset(resourceName);
    if (modelResource)
        return modelResource;

    // try to load
    ModelAsset modeldata {resourceName};
    if (modeldata.Load())
    {
        modelResource = new ModelAsset {resourceName};
        modelResource->Exchange(modeldata);
    }

    if (modelResource)
    {
        // add to cache
        mModelAssetsMap[resourceName] = modelResource;
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load model '%s'", resourceName.c_str());
    }
    return modelResource;
}