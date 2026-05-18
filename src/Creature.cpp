#include "stdafx.h"
#include "Creature.h"
#include "CreatureController.h"
#include "CreatureManager.h"
#include "Scene.h"
#include "MeshAssetManager.h"
#include "PhysicsObject.h"
#include "GameWorld.h"
#include "Physics.h"

void Creature::ConfigureInstance(EntityUid instanceUid, CreatureController* controller, CreatureDefinition* definition, ePlayerID owner)
{
    mInstanceUid = instanceUid;

    mOwnerID = owner;
    cxx_assert(mOwnerID != ePlayerID_Null);

    cxx_assert((mDefinition == nullptr) && definition);
    mDefinition = definition;

    cxx_assert(mController == nullptr);
    mController = controller;

    if (mController)
    {
        mController->ConfigureInstance(this);
    }
}

void Creature::SpawnInstance()
{
    cxx_assert(!mLifecycleFlags.mWasSpawned);
    cxx_assert(!mLifecycleFlags.mWasDespawned);
    cxx_assert(!mLifecycleFlags.mWasDeleted);

    if (mLifecycleFlags.mWasSpawned) return;

    mOwnHandle = GetCreatureManager().FindCreature(mInstanceUid);

    EnableMeshObject(true);
    
    // configure locomotion
    {
        mLocomotion.ResetToDefaults();
        mLocomotion.SetArriveSpeed(mDefinition->mSpeed);
    }

    EnablePhysics(true);

    if (mController)
    {
        mController->SpawnInstance();
    }

    mLifecycleFlags.mWasSpawned = true;
}

void Creature::DespawnInstance()
{
    cxx_assert(mLifecycleFlags.mWasSpawned);

    if (mLifecycleFlags.mWasDespawned) return;

    if (mController)
    {
        mController->DespawnInstance();
    }

    mOwnerID = ePlayerID_Null;
    mOwnHandle = {};

    mCurrentActivity.reset();
    mRequestActivity.reset();

    mLocomotion.ResetToDefaults();
    mLocomotion.ClearGoals();

    EnablePhysics(false);
    EnableMeshObject(false);

    mLifecycleFlags.mWasDespawned = true;
}

void Creature::UpdateLogic(float stepDeltaTime)
{
    if (mController)
    {
        mController->UpdateLogic(stepDeltaTime);
    }
}

void Creature::UpdatePhysics(float stepDeltaTime)
{
    // process locomotion
    if (mLocomotion.HasGoals())
    {
        Locomotion::VelocitiesResult velocities = mLocomotion.ProcessGoals(stepDeltaTime, GetTransform());
        // notify self
        if (mLocomotion.HasGoals())
        {
            Notify(EntityNotification::ForLocoApplyVelocities(velocities.mLinearVelocity, velocities.mAngularVelocity));
        }
        else
        {
            Notify(EntityNotification::ForLocoClearVelocities());
        }
    }
}

void Creature::UpdateFrame(float deltaTime)
{
    // process animations
    mAnimator.UpdateFrame(deltaTime);

    if (mController)
    {
        mController->UpdateFrame(deltaTime);
    }
}

void Creature::SetPosition(const glm::vec3& position)
{
    mTransform.mPosition = position;
    if (mMeshObject)
    {
        mMeshObject->SetPosition(mTransform.mPosition);
    }

    // sync physics
    if (mPhysicsObject)
    {
        mPhysicsObject->SetTransform(mTransform);
    }
}

void Creature::SetPosition(const glm::vec2& position)
{
    const glm::vec3 position3d { position.x, mTransform.mPosition.y, position.y };
    SetPosition(position3d);
}

void Creature::SnapPositionToFloor(bool withRespectToMeshBounds)
{
    float floorHeight = GetGameWorld().GetGameMap().GetFloorHeightAt(mTransform.mPosition);

    if (withRespectToMeshBounds)
    {
        float extraOffsetFromFloor = 0.0f;
        // get the offset between the y position and the mesh's bottom aabbox
        const cxx::aabbox& meshLocalBounds = GetMeshLocalBounds();
        if (meshLocalBounds.is_valid())
        {
            extraOffsetFromFloor = (0.0f - meshLocalBounds.mMin.y);
        }
        floorHeight += extraOffsetFromFloor;
    }

    if (cxx::eps_equals(floorHeight, mTransform.mPosition.y))
        return;

    const glm::vec3 newPosition { mTransform.mPosition.x, floorHeight, mTransform.mPosition.z };
    SetPosition(newPosition);
}

void Creature::SetOrientation(cxx::angle_t orientation)
{
    mTransform.mOrientation = orientation;
    if (mMeshObject)
    {
        mMeshObject->ResetOrientation();
        mMeshObject->RotateAroundAxis(WorldAxes::Y, mTransform.mOrientation);
    }

    // sync physics
    if (mPhysicsObject)
    {
        mPhysicsObject->SetTransform(mTransform);
    }
}

MapPoint2D Creature::GetTilePosition() const
{
    const glm::vec3 position = GetPosition();
    return MapUtils::ComputeTileFromPosition(position);
}

const cxx::aabbox& Creature::GetMeshLocalBounds() const
{
    if (mMeshObject)
    {
        return mMeshObject->GetLocalBounds();
    }
    static const cxx::aabbox nullBounds;
    return nullBounds;
}

const cxx::aabbox& Creature::GetMeshWorldBounds() const
{
    if (mMeshObject)
    {
        mMeshObject->ComputeTransformation();
        return mMeshObject->GetWorldBounds();
    }
    static const cxx::aabbox nullBounds;
    return nullBounds;
}

void Creature::MarkDeleted()
{
    mLifecycleFlags.mWasDeleted = true;
}

void Creature::OnRecycle()
{
    Entity::OnRecycle();

    mController = nullptr;
    mDefinition = nullptr;

    cxx_assert(mPhysicsObject == nullptr);
    mPhysicsObject = nullptr;

    mLocomotion.ResetToDefaults();
    mLocomotion.ClearGoals();

    mAnimator.Clear();
}

void Creature::EnableMeshObject(bool isEnabled)
{
    bool wasEnabled = (mMeshObject != nullptr);
    if (wasEnabled == isEnabled) return;

    // cleanup
    if (wasEnabled)
    {
        mMeshObject.reset();
        mAnimator.Clear();

        return;
    }

    // init mesh
    mMeshObject = GetScene().CreateAnimatingMesh();
    cxx_assert(mMeshObject);

    if (mMeshObject)
    {
        // sync mesh transformation with object
        mMeshObject->ResetTransformation();
        mMeshObject->SetPosition(mTransform.mPosition);
        mMeshObject->RotateAroundAxis(WorldAxes::Y, mTransform.mOrientation);
    }

    // init animator
    if (mMeshObject)
    {
        mMeshObject->SetObjectActive(true);
    }
    mAnimator.Configure(mMeshObject.get());
}

void Creature::EnablePhysics(bool isEnabled)
{
    bool wasEnabled = (mPhysicsObject != nullptr);
    if (wasEnabled == isEnabled) return;

    if (wasEnabled)
    {
        // cleanup
        GetGameWorld().GetPhysics().DetachUser(this);
        mPhysicsObject = nullptr;

        return;
    }

    // init physics
    GetGameWorld().GetPhysics().AttachUser(this);

    mPhysicsObject = GetGameWorld().GetPhysics().GetPhysicsObject(this);
    cxx_assert(mPhysicsObject);
    mPhysicsObject->ClearAngularVelocity();
    mPhysicsObject->ClearLinearVelocity();
}

void Creature::Notify(const EntityNotification& notificationData)
{
    if (WasDeleted()) return;

    if (notificationData.mID == EntityNotification::eID_SyncWithPhysicsTransform)
    {
        if (mPhysicsObject)
        {
            mTransform = mPhysicsObject->GetTransform();
            if (mMeshObject)
            {
                mMeshObject->SetPosition(mTransform.mPosition);
                mMeshObject->ResetOrientation();
                mMeshObject->RotateAroundAxis(WorldAxes::Y, mTransform.mOrientation);
            }
        }
        return;
    }

    if (mController)
    {
        mController->HandleNotification(notificationData);
    }

    if (notificationData.mID == EntityNotification::eID_LocoApplyVelocities)
    {
        if (mPhysicsObject)
        {
            mPhysicsObject->SetLinearVelocity(notificationData.mLocoVelocities.mLinear);
            mPhysicsObject->SetAngularVelocity(notificationData.mLocoVelocities.mAngular);
        }
        return;
    }

    if (notificationData.mID == EntityNotification::eID_LocoClearVelocities)
    {
        if (mPhysicsObject)
        {
            mPhysicsObject->ClearLinearVelocity();
            mPhysicsObject->ClearAngularVelocity();
        }
        return;
    }
}

void Creature::ClearCurrentActivity()
{
    mCurrentActivity.reset();
}

void Creature::ClearRequestActivity()
{
    mRequestActivity.reset();
}

void Creature::SwitchToRequestActivity()
{
    mRequestActivity.swap(mCurrentActivity);
    mRequestActivity.reset();
}

void Creature::CancelCurrentActivity()
{
    if (CreatureActivity* currentActivity = GetCurrentActivity())
    {
        CreatureActivityUtils::SetCancellationStatus(*currentActivity);
    }
}

