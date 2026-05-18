#include "stdafx.h"
#include "PhysicsObject.h"

void PhysicsObject::Configure(Entity* entity, const EntityTransform& entityTransform)
{
    mEntity = entity;
    cxx_assert(mEntity);

    mCurrTransform = entityTransform;
    mNextTransform = entityTransform;
    mPrevTransform = entityTransform;

    mLinearVelocity = {};
    mAngularVelocity = {};
}

void PhysicsObject::SetLinearVelocity(const glm::vec2& velocity)
{
    mLinearVelocity = velocity;
}

void PhysicsObject::AddLinearVelocity(const glm::vec2& velocity)
{
    mLinearVelocity += velocity;
}

void PhysicsObject::ClearLinearVelocity()
{
    mLinearVelocity = {};
}

bool PhysicsObject::HasLinearVelocity() const
{
    return !cxx::eps_equals_zero(mLinearVelocity.x) || !cxx::eps_equals_zero(mLinearVelocity.y);
}

void PhysicsObject::SetAngularVelocity(float velocity)
{
    mAngularVelocity = velocity;
}

void PhysicsObject::AddAngularVelocity(float velocity)
{
    mAngularVelocity += velocity;
}

void PhysicsObject::ClearAngularVelocity()
{
    mAngularVelocity = {};
}

bool PhysicsObject::HasAngularVelocity() const
{
    return !cxx::eps_equals_zero(mAngularVelocity);
}

void PhysicsObject::SetTransform(const EntityTransform& entityTransform)
{
    mPrevTransform = entityTransform;
    mNextTransform = entityTransform;
    mCurrTransform = entityTransform; 
}

void PhysicsObject::OnRecycle()
{
    mLinearVelocity = {};
    mAngularVelocity = {};
    mPrevTransform = {};
    mNextTransform = {};
    mCurrTransform = {};
    mEntity = nullptr;
}
