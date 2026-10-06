#include "stdafx.h"
#include "EntityComponents.h"
#include "Physics.h"
#include "AnimatingMeshObject.h"
#include "Scene.h"
#include "SimplePool.h"
#include "Locomotion.h"
#include "Animator.h"
#include "Entity.h"

//////////////////////////////////////////////////////////////////////////

MoneyComponent::MoneyComponent(Entity* entity)
{
    // leave ctor
}

//////////////////////////////////////////////////////////////////////////

PhysicsComponent::PhysicsComponent(Entity* entity)
{
    mPhysicsObject = gPhysics.CreatePhysicsObject(entity);
    cxx_assert(mPhysicsObject);
}

PhysicsComponent::~PhysicsComponent()
{
    mPhysicsObject.reset();
}

bool PhysicsComponent::IsEnabled() const
{
    return true;
}

void PhysicsComponent::SetEnabled(bool isEnabled)
{
    // todo
}

//////////////////////////////////////////////////////////////////////////

LocomotionComponent::LocomotionComponent(Entity* entity)
{
    static SimplePool<Locomotion> objectsPool = (
        [](Locomotion* object)
        {
            object->OnRecycle();
        });

    Locomotion* objectPtr = objectsPool.Acquire();
    mLoco = std::move(LocomotionPtr (objectPtr, [](Locomotion* object)
        {
            if (object)
            {
                objectsPool.Return(object);
            }
        }));
}

LocomotionComponent::~LocomotionComponent()
{
    mLoco.reset();
}

//////////////////////////////////////////////////////////////////////////

AnimatingMeshComponent::AnimatingMeshComponent(Entity* entity)
{
    mMeshObject = gScene.CreateAnimatingMesh();
    cxx_assert(mMeshObject);

    const EntityTransform& entityTransform = entity->GetTransform();

    // sync mesh transformation with object
    mMeshObject->ResetTransformation();
    mMeshObject->SetPosition(entityTransform.mPosition);
    mMeshObject->RotateAroundAxis(WorldAxes::Y, entityTransform.mOrientation);

    mMeshObject->SetHighlighted(entity->IsHighlighted());
    mMeshObject->SetOwnerEntity(entity->GetOwnHandle());
    mMeshObject->SetObjectActive(true);
}

AnimatingMeshComponent::~AnimatingMeshComponent()
{
    mMeshObject.reset();
}

bool AnimatingMeshComponent::IsEnabled() const
{
    return mMeshObject->IsObjectActive();
}

void AnimatingMeshComponent::SetEnabled(bool isEnabled)
{
    mMeshObject->SetObjectActive(isEnabled);
}

//////////////////////////////////////////////////////////////////////////

AnimatorComponent::AnimatorComponent(Entity* entity)
{
    static SimplePool<Animator> objectsPool = (
        [](Animator* object)
        {
            object->OnRecycle();
        });

    Animator* objectPtr = objectsPool.Acquire();
    mAnimator = std::move(AnimatorPtr (objectPtr, [](Animator* object)
        {
            if (object)
            {
                objectsPool.Return(object);
            }
        }));
}

AnimatorComponent::~AnimatorComponent()
{
    mAnimator.reset();
}

//////////////////////////////////////////////////////////////////////////