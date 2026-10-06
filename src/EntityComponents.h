#pragma once

//////////////////////////////////////////////////////////////////////////

#include "PhysicsDefs.h"
#include "SceneDefs.h"

//////////////////////////////////////////////////////////////////////////

// Entity Components are optional data containers of an Entity

//////////////////////////////////////////////////////////////////////////

struct MoneyComponent
{
public:
    MoneyComponent(Entity* entity);
public:
    long mAmount = 0;
    long mCapacity = 0;
};

//////////////////////////////////////////////////////////////////////////

struct PhysicsComponent
{
public:
    PhysicsComponent(Entity* entity);
    ~PhysicsComponent();

    bool IsEnabled() const;
    void SetEnabled(bool isEnabled);
public:
    PhysicsObjectPtr mPhysicsObject;
};

//////////////////////////////////////////////////////////////////////////

struct LocomotionComponent
{
public:
    LocomotionComponent(Entity* entity);
    ~LocomotionComponent();
public:
    LocomotionPtr mLoco;
};

//////////////////////////////////////////////////////////////////////////

struct AnimatingMeshComponent
{
public:
    AnimatingMeshComponent(Entity* entity);
    ~AnimatingMeshComponent();

    bool IsEnabled() const;
    void SetEnabled(bool isEnabled);
public:
    AnimatingMeshObjectPtr mMeshObject;
};

//////////////////////////////////////////////////////////////////////////

struct AnimatorComponent
{
public:
    AnimatorComponent(Entity* entity);
    ~AnimatorComponent();
public:
    AnimatorPtr mAnimator;
};

//////////////////////////////////////////////////////////////////////////