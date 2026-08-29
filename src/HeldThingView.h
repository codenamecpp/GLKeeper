#pragma once

//////////////////////////////////////////////////////////////////////////

#include "SceneDefs.h"
#include "AssetDefs.h"

//////////////////////////////////////////////////////////////////////////

// in-hand creature/object view

class HeldThingView
{
public:
    HeldThingView() = default;
    ~HeldThingView();

    void SetHeldThing(EntityHandle entHandle);
    bool HasHeldThing() const;
    bool HasHeldThing(EntityHandle entHandle) const;
    void SetHeldNothing();
    void SetScreenPosition(const Point2D& screenPosition);

private:
    void SetHeldThing(GameObjectDefinition* definition);
    void SetHeldThing(CreatureDefinition* definition);
    void SetMeshAsset(const std::string& assetName);

private:
    EntityHandle mEntityHandle;
    MeshAsset* mMeshAsset = nullptr;
    cxx::uniqueptr<AnimatingMeshObject> mMeshObject;
    cxx::uniqueptr<RenderView> mRenderView;
};

//////////////////////////////////////////////////////////////////////////