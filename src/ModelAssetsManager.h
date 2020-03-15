#pragma once

#include "ResourceDefs.h"

// models manager class
class ModelAssetsManager: public cxx::noncopyable
{
public:

    // setup manager internal resources, returns false on error
    bool Initialize();
    void Deinit();

    // get previously loaded kmf model from cache
    // @param resourceName: Kmf model resource name without extension
    ModelAsset* FindModelAsset(const std::string& resourceName) const;

    // load kmf model
    // @param resourceName: Kmf model resource name without extension
    // @returns null if kmf model cannot be loaded
    ModelAsset* LoadModelAsset(const std::string& resourceName);

private:
    using ModelAssetsMap = std::map<std::string, ModelAsset*, cxx::icase_less>;
    ModelAssetsMap mModelAssetsMap;
};

extern ModelAssetsManager gModelsManager;