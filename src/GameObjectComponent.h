#pragma once

//////////////////////////////////////////////////////////////////////////

namespace details
{
    template<typename TGameObjectComponent>
    struct gameobject_component_traits
    {
    };

} // namespace details

#define decl_gameobject_component(class_name, component_typeid) \
    namespace details \
    { \
        template<> \
        struct gameobject_component_traits<class_name> \
        { \
            inline static eGameObjectComponent GetComponentType() \
            { \
                return component_typeid; \
            } \
        }; \
    }

#define gameobject_component_typeid(class_name) \
    \
    details::gameobject_component_traits<class_name>::GetComponentType()

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