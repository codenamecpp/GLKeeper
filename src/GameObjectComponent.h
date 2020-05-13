#pragma once

//////////////////////////////////////////////////////////////////////////

// put this macro inside each component class declaration
#define GAMEOBJECT_COMPONENT(componentid) \
    public: \
        static eGameObjectComponent GetComponentType() \
        { \
            return componentid; \
        }

//////////////////////////////////////////////////////////////////////////

// base class of any game object component
class GameObjectComponent: public cxx::noncopyable
{
public:
    const eGameObjectComponent mComponentType;
    GameObject* const mGameObject;

public:
    virtual ~GameObjectComponent()
    {
    }
    // process scene update frame
    // @param deltaTime: Time since last update
    virtual void UpdateFrame(float deltaTime) 
    { 
    }
    // issue draw call
    // @param renderContext: Scene render context
    virtual void RenderFrame(SceneRenderContext& renderContext) 
    { 
    }
protected:
    // base component does not meant to be instantiated directly
    GameObjectComponent(eGameObjectComponent componentType, GameObject* sceneObject)
        : mComponentType(componentType)
        , mGameObject(sceneObject)
    {
        debug_assert(mComponentType < eGameObjectComponent_Count);
        debug_assert(mGameObject);
    }
};