#pragma once

#include "GameDefs.h"
#include "RenderDefs.h"

class GameObjectComponent: public cxx::noncopyable
{
public:
    const eGameObjectComponent mComponentType;

    GameObject* mGameObject;

public:
    GameObjectComponent(eGameObjectComponent componentType, GameObject* sceneObject)
        : mComponentType(componentType)
        , mGameObject(sceneObject)
    {
        debug_assert(mComponentType < eGameObjectComponent_Count);
    }

    virtual ~GameObjectComponent()
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
            case eGameObjectComponent_Transform:
            case eGameObjectComponent_TerrainMesh:
                return false;

            case eGameObjectComponent_WaterLavaMesh:
            case eGameObjectComponent_AnimatingModel:
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
            case eGameObjectComponent_Transform:
                return false;

            case eGameObjectComponent_TerrainMesh:
            case eGameObjectComponent_WaterLavaMesh:
            case eGameObjectComponent_AnimatingModel:
                return true;
        }
        debug_assert(false);
        return false;
    }
};