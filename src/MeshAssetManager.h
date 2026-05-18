#pragma once

//////////////////////////////////////////////////////////////////////////

#include "MeshAsset.h"

//////////////////////////////////////////////////////////////////////////

class MeshAssetManager : public cxx::noncopyable
{
public:
    void Initialize();
    void Shutdown();

    struct LoadParams
    {
        // will create fallback mesh if loading fails for some reason
        bool mInitDefaultOnFail = true;
    };

    // loads new mesh if it is not loaded yet
    // will always return true if mInitDefaultOnFail is set to true (operation never fails)
    // will not init render data automatically
    // returns nullptr if name is empty
    MeshAsset* LoadMesh(const std::string& name, const LoadParams& params = {});

    // returns loaded mesh or nullptr if it not found
    MeshAsset* FindMesh(const std::string& name) const;

    // returns already loaded mesh (params are ignored) or loads new mesh otherwise
    // returns nullptr if name is empty
    MeshAsset* GetMesh(const std::string& name, const LoadParams& params = {});

private:

    using ResourcesMap = std::map<std::string, std::unique_ptr<MeshAsset>, cxx::icase_string_less>;
    ResourcesMap mResourcesMap;
};

//////////////////////////////////////////////////////////////////////////

extern MeshAssetManager gMeshAssetManager;

//////////////////////////////////////////////////////////////////////////