#include "pch.h"
#include "ModelsManager.h"
#include "FileSystem.h"
#include "Console.h"
#include "KmfModel_Data.h"

ModelsManager gModelsManager;

bool ModelsManager::Initialize()
{
    return true;
}

void ModelsManager::Deinit()
{
    mModelsMap.clear();
}

KmfModel_Data* ModelsManager::FindModel(const std::string& resourceName) const
{
    auto models_iterator = mModelsMap.find(resourceName);
    if (models_iterator != mModelsMap.end())
    {
        return models_iterator->second;
    }
    return nullptr;
}

KmfModel_Data* ModelsManager::LoadModel(const std::string& resourceName)
{
    KmfModel_Data* modelResource = FindModel(resourceName);
    if (modelResource)
        return modelResource;

    // try to load
    if (BinaryInputStream* inputStream = gFileSystem.OpenDataFile(resourceName))
    {
        KmfModel_Data modeldata {resourceName};
        if (modeldata.LoadDataFromStream(inputStream))
        {
            modelResource = new KmfModel_Data {resourceName};
            modelResource->Exchange(modeldata);
        }
        gFileSystem.CloseFileStream(inputStream);
    }

    if (modelResource)
    {
        // add to cache
        mModelsMap[resourceName] = modelResource;
    }
    else
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load model '%s'", resourceName.c_str());
    }
    return modelResource;
}