#pragma once

#include "SceneDefs.h"

// generic scene object component
class SceneObjectComponent: public cxx::noncopyable
{
public:
    const eSceneObjectComponent mComponentType;

    SceneObject* mSceneObject;

public:
    SceneObjectComponent(eSceneObjectComponent componentType, SceneObject* sceneObject)
        : mComponentType(componentType)
        , mSceneObject(sceneObject)
    {
        debug_assert(mComponentType < eSceneObjectComponent_Count);
    }

    virtual ~SceneObjectComponent()
    {
    }
    
    // process scene update frame
    // @param deltaTime: Time since last update
    virtual void UpdateFrame(float deltaTime) { }

    // issue draw call
    // @param renderContext: Scene render context
    virtual void RenderFrame(SceneRenderContext& renderContext) { }

    // whether component should update
    inline bool IsUpdatableComponent() const
    {
        switch (mComponentType)
        {
        case eSceneObjectComponent_Transform:
        case eSceneObjectComponent_TerrainMesh:
            return false;

        case eSceneObjectComponent_WaterLavaMesh:
        case eSceneObjectComponent_AnimatingModel:
            return true;
        }
        debug_assert(false);
        return false;
    }

    // whether component should render
    inline bool IsRenderableComponent() const
    {
        switch (mComponentType)
        {
            case eSceneObjectComponent_Transform:
                return false;

            case eSceneObjectComponent_TerrainMesh:
            case eSceneObjectComponent_WaterLavaMesh:
            case eSceneObjectComponent_AnimatingModel:
                return true;
        }
        debug_assert(false);
        return false;
    }
};