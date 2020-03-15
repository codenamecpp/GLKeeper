#pragma once

#include "SceneDefs.h"
#include "RenderDefs.h"

// list for collecting scene entities which will be rendered on current frame
class SceneRenderList
{
public:
    SceneRenderList() = default;

    static const int MaxElements = 16383;

    template<typename TSceneObject>
    inline void RegisterObject(eRenderPass renderPass, TSceneObject* sceneObject)
    {
        debug_assert(sceneObject);
        if (renderPass == eRenderPass_Opaque)
        {
            if (mOpaqueElementsCount == MaxElements)
                return;

            mOpaqueElements[mOpaqueElementsCount++].Set(sceneObject);
            return;
        }

        if (renderPass == eRenderPass_Translucent)
        {
            if (mTranslucentElementsCount == MaxElements)
                return;

            mTranslucentElements[mTranslucentElementsCount++].Set(sceneObject);
            return;
        }
        debug_assert(false);
    }

    // discard all previously collected entities
    inline void Clear()
    {
        mOpaqueElementsCount = 0;
        mTranslucentElementsCount = 0;
    }

public:

    // single registered scene object
    struct ListElementStruct
    {
    public:
        inline void Set(AnimatingModel* animatingModel)
        {
            Clear();
            mAnimatingModel = animatingModel;
            mObject = (SceneObject*) animatingModel;
        }
        inline void Set(TerrainMesh* terrainMesh)
        {
            Clear();
            mTerrainMesh = terrainMesh;
            mObject = (SceneObject*) terrainMesh;
        }
        inline void Clear()
        {
            mAnimatingModel = nullptr;
            mTerrainMesh = nullptr;
            mObject = nullptr;
        }
    public:
        SceneObject* mObject = nullptr; // cannot be null
        AnimatingModel* mAnimatingModel = nullptr;
        TerrainMesh* mTerrainMesh = nullptr;
    };

    ListElementStruct mOpaqueElements[MaxElements];
    int mOpaqueElementsCount = 0;

    ListElementStruct mTranslucentElements[MaxElements];
    int mTranslucentElementsCount = 0;
};
