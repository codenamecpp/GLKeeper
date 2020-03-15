#pragma once

#include "SceneDefs.h"
#include "RenderDefs.h"
#include "SceneObject.h"

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

    // sort opaque objects by render program
    inline void SortOpaquesByObjectType()
    {
        if (mOpaqueElementsCount > 2)
        {
            std::sort(mOpaqueElements, mOpaqueElements + mOpaqueElementsCount, 
                [](const ListElementStruct& lhs, const ListElementStruct& rhs)
            {
                // terrain meshes
                if (lhs.mTerrainMesh != rhs.mTerrainMesh) return (rhs.mTerrainMesh == nullptr);

                // water and lava
                if (lhs.mWaterLavaMesh != rhs.mWaterLavaMesh) return (rhs.mWaterLavaMesh == nullptr);

                // animating models
                if (lhs.mAnimatingModel != rhs.mAnimatingModel) return (rhs.mAnimatingModel == nullptr);

                return lhs.mObject < rhs.mObject;
            });
        }
    }

    // sort translucent object by distance to camera
    inline void SortTranslucentsByDistanceToCamera()
    {
        if (mTranslucentElementsCount > 2)
        {
            std::sort(mTranslucentElements, mTranslucentElements + mTranslucentElementsCount, 
                [](const ListElementStruct& lhs, const ListElementStruct& rhs)
            {
                return lhs.mObject->mDistanceToCameraSquared < rhs.mObject->mDistanceToCameraSquared;
            });
        }
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
        inline void Set(WaterLavaMesh* waterLavaMesh)
        {
            Clear();
            mWaterLavaMesh = waterLavaMesh;
            mObject = (SceneObject*) waterLavaMesh;
        }
        inline void Clear()
        {
            mAnimatingModel = nullptr;
            mTerrainMesh = nullptr;
            mObject = nullptr;
            mWaterLavaMesh = nullptr;
        }
    public:
        SceneObject* mObject = nullptr; // cannot be null
        AnimatingModel* mAnimatingModel = nullptr;
        TerrainMesh* mTerrainMesh = nullptr;
        WaterLavaMesh* mWaterLavaMesh = nullptr;
    };

    ListElementStruct mOpaqueElements[MaxElements];
    int mOpaqueElementsCount = 0;

    ListElementStruct mTranslucentElements[MaxElements];
    int mTranslucentElementsCount = 0;
};
