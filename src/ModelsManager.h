#pragma once

// forwards
class KmfModel_Data;

// models manager class
class ModelsManager: public cxx::noncopyable
{
public:

    // setup manager internal resources, returns false on error
    bool Initialize();
    void Deinit();

    // get previously loaded kmf model from cache
    // @param resourceName: Kmf model resource name
    KmfModel_Data* FindModel(const std::string& resourceName) const;

    // load kmf model
    // @param resourceName: Kmf model resource name
    // @returns null if kmf model cannot be loaded
    KmfModel_Data* LoadModel(const std::string& resourceName);

private:
    using ModelsMap = std::map<std::string, KmfModel_Data*, cxx::icase_less>;
    ModelsMap mModelsMap;
};

extern ModelsManager gModelsManager;