#include "stdafx.h"
#include "Physics.h"
#include "SimplePool.h"
#include "GameWorld.h"
#include "Entity.h"

void Physics::EnterWorld()
{
    mSimulationStepDelta = gTime.GetFixedDelta(eFixedClock::GamePhysics);
    cxx_assert(mSimulationStepDelta > 0.0f);
    mInterpolationTime = 0.0f;
    mEntities.reserve(1024);
    mObjects.reserve(1024);
    mInterpolateTransforms.reserve(1024);
}

void Physics::ClearWorld()
{
    mEntities.clear();
    mObjects.clear();
    mInterpolateTransforms.clear();
}

void Physics::UpdateFrame(float deltaTime)
{
    mInterpolationTime += deltaTime;

    if (mInterpolateTransforms.empty())
        return;

    const float t = mInterpolationTime / mSimulationStepDelta;
    if ((t > 0.0f) && (t < 1.0f))
    {
        for (PhysicsObject* roller: mInterpolateTransforms)
        {
            InterpolationStep(roller, t);
        }
    }
}

void Physics::UpdatePhysics(float stepDeltaTime)
{
    mInterpolationTime = 0.0f;
    
    // force syns transform
    if (!mInterpolateTransforms.empty())
    {
        for (PhysicsObject* roller: mInterpolateTransforms)
        {
            InterpolationStep(roller, 1.0f);
        }
        mInterpolateTransforms.clear();
    }

    // process simulation
    for (PhysicsObject* roller: mObjects)
    {
        SimulationStep(roller);
        ResetVelocities(roller);
    }

    // force syns transform
    if (!mInterpolateTransforms.empty())
    {
        for (PhysicsObject* roller: mInterpolateTransforms)
        {
            InterpolationStep(roller, 0.0f);
        }
    }
}

bool Physics::LoadScenario(const ScenarioDefinition& scenarioDef)
{
    return true;
}

void Physics::AttachUser(Entity* entity)
{
    if ((entity == nullptr) || GetPhysicsObject(entity))
    {
        cxx_assert(false);
        return;
    }

    EntityEntry& entityEntry = mEntities.emplace_back();
    entityEntry.first = entity;
    entityEntry.second = CreatePhysicsObject();
    entityEntry.second->Configure(entity, entity->GetTransform());

    mObjects.push_back(entityEntry.second.get());
}

void Physics::DetachUser(Entity* entity)
{
    auto ent_it = std::find_if(mEntities.begin(), mEntities.end(), [entity](const EntityEntry& entry) 
        { 
            return entry.first == entity; 
        });
    if (ent_it == mEntities.end())
    {
        cxx_assert(false);
        return;
    }
    cxx::erase(mObjects, ent_it->second.get());
    cxx::erase(mInterpolateTransforms, ent_it->second.get());
    mEntities.erase(ent_it);
}

PhysicsObject* Physics::GetPhysicsObject(Entity* entity) const
{
    auto object_iter = std::find_if(mEntities.begin(), mEntities.end(), [entity](const EntityEntry& entry)
        {
            return entry.first == entity;
        });
    PhysicsObject* physicsObject = nullptr;
    if (object_iter != mEntities.end())
    {
        physicsObject = object_iter->second.get();
    }
    return physicsObject;
}

cxx::uniqueptr<PhysicsObject> Physics::CreatePhysicsObject()
{
    static SimplePool<PhysicsObject> objectsPool = (
        [](PhysicsObject* object)
        {
            object->OnRecycle();
        });

    PhysicsObject* objectPtr = objectsPool.Acquire();
    return std::move(cxx::uniqueptr<PhysicsObject> (objectPtr, [](PhysicsObject* object)
    {
        if (object)
        {
            objectsPool.Return(object);
        }
    }));
}

void Physics::InterpolationStep(PhysicsObject* object, float t)
{
    if (t == 0.0f)
    {
        object->mCurrTransform = object->mPrevTransform;
    }
    else if (t == 1.0f)
    {
        object->mCurrTransform = object->mNextTransform;
    }
    else
    {
        // rotation
        {
            float diff = (object->mNextTransform.mOrientation - object->mPrevTransform.mOrientation).to_radians();
            if (!cxx::eps_equals_zero(diff))
            {
                float interpolated = cxx::wrap_angle_to_pi(object->mPrevTransform.mOrientation.to_radians() + cxx::wrap_angle_to_pi(diff) * t);
                object->mCurrTransform.mOrientation = cxx::angle_t::from_radians(interpolated);
            }
        }
        // movement
        {
            object->mCurrTransform.mPosition = glm::lerp(object->mPrevTransform.mPosition, object->mNextTransform.mPosition, t);
        }
    }

    // notify
    if (object->mEntity)
    {
        object->mEntity->Notify(EntityNotification::ForSyncWithPhysics());
    }
}

void Physics::SimulationStep(PhysicsObject* object)
{
    bool simulated = false;

    object->mPrevTransform = object->mCurrTransform;

    // rotation
    bool isRotating = !cxx::eps_equals_zero(object->GetAngularVelocity());
    if (isRotating)
    {
        float turn_step = cxx::wrap_angle_to_pi(mSimulationStepDelta * object->GetAngularVelocity());
        float turn_angle = cxx::wrap_angle_to_pi(object->mCurrTransform.mOrientation.to_radians() + turn_step);
        object->mPrevTransform.mOrientation = object->mCurrTransform.mOrientation;
        object->mNextTransform.mOrientation = cxx::angle_t::from_radians(turn_angle);

        simulated = true;
    }

    // movement
    bool isMoving = !cxx::eps_equals_zero(object->GetLinearSpeed2());
    if (isMoving)
    {
        const glm::vec2& linearVelocity = object->GetLinearVelocity();
        object->mNextTransform.mPosition.x = object->mCurrTransform.mPosition.x + linearVelocity.x * mSimulationStepDelta;
        object->mNextTransform.mPosition.z = object->mCurrTransform.mPosition.z + linearVelocity.y * mSimulationStepDelta;
        object->mNextTransform.mPosition.y = GetGameWorld().GetGameMap().GetFloorHeightAt(object->mNextTransform.mPosition);

        simulated = true;

        // todo: add collisions
    }

    // gravity
    if (!isMoving)
    {
        float prevFloorHeight = object->mCurrTransform.mPosition.y;
        float currFloorHeight = GetGameWorld().GetGameMap().GetFloorHeightAt(object->mCurrTransform.mPosition);
        if (!cxx::eps_equals(currFloorHeight, prevFloorHeight))
        {
            object->mNextTransform.mPosition.y = currFloorHeight;
            simulated = true;
        }
    }

    // queue
    if (simulated)
    {
        mInterpolateTransforms.push_back(object);
    }
}

void Physics::ResetVelocities(PhysicsObject* object)
{
    object->ClearAngularVelocity();
    object->ClearLinearVelocity();
}
