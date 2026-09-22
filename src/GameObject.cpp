#include "stdafx.h"
#include "GameObject.h"
#include "GameWorld.h"
#include "AnimatingMeshObject.h"
#include "MeshAssetManager.h"
#include "GameMain.h"
#include "Physics.h"
#include "GameObjectManager.h"
#include "Scene.h"
#include "GameMap.h"

void GameObject::ConfigureInstance(EntityUid instanceUid, GameObjectController* objectController, GameObjectDefinition* objectDef)
{
    mInstanceUid = instanceUid;

    cxx_assert((mDefinition == nullptr) && objectDef);
    mDefinition = objectDef;

    cxx_assert(mController == nullptr);
    mController = objectController;

    mCurrentState = mDefinition->mStartState;

    // controller is optional for simpler objects
    if (mController)
    {
        mController->ConfigureInstance(this);
    }
}

void GameObject::SpawnInstance()
{
    cxx_assert(!mEntityFlags.mWasSpawned);
    cxx_assert(!mEntityFlags.mWasDespawned);
    cxx_assert(!mEntityFlags.mWasDeleted);

    if (mEntityFlags.mWasSpawned) 
        return;

    mEntityFlags.mWasSpawned = true;

    mOwnHandle = gGameObjectManager.FindObject(mInstanceUid);

    // configure locomotion
    {
        mLocomotion.ResetToDefaults();
        mLocomotion.ClearGoals();
        mLocomotion.SetArriveSpeed(mDefinition->mSpeed);
    }

    // init mesh
    if (mDefinition->mResourceMesh.IsDefined())
    {
        ConfigureMeshResource(mDefinition->mResourceMesh);
    }

    if (mController)
    {
        mController->SpawnInstance();
    }
}

void GameObject::DespawnInstance()
{
    cxx_assert(mEntityFlags.mWasSpawned);

    if (mEntityFlags.mWasDespawned) 
        return;

    if (mController)
    {
        mController->DespawnInstance();
    }

    mLocomotion.ClearGoals();
    mLocomotion.ResetToDefaults();

    EnablePhysics(false);
    ShowMesh(false);

    mOwnHandle = {};

    mEntityFlags.mWasDespawned = true;
}

void GameObject::InitMesh()
{
    if (mMeshObject)
        return;

    mMeshObject = gScene.CreateAnimatingMesh();
    cxx_assert(mMeshObject);

    if (mMeshObject)
    {
        // sync mesh transformation with object
        mMeshObject->ResetTransformation();
        mMeshObject->SetPosition(mTransform.mPosition);
        mMeshObject->RotateAroundAxis(WorldAxes::Y, mTransform.mOrientation);

        // params
        mMeshObject->SetHighlighted(IsHighlighted());
        mMeshObject->SetOwnerEntity(GetOwnHandle());
        mMeshObject->SetObjectActive(true);    
    }
}

void GameObject::FreeMesh()
{
    mMeshObject.reset();
}

void GameObject::ShowMesh(bool isEnabled)
{
    if (mMeshObject)
    {
        bool wasEnabled = mMeshObject->IsObjectActive();
        if (wasEnabled != isEnabled)
        {
            mMeshObject->SetObjectActive(isEnabled);
        }
    }
}

void GameObject::SetPosition(const glm::vec3& position)
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

void GameObject::SetPosition(const glm::vec2& position)
{
    const glm::vec3 position3d { position.x, mTransform.mPosition.y, position.y };
    SetPosition(position3d);
}

void GameObject::SnapPositionToFloor(bool withRespectToMeshBounds)
{
    float floorHeight = gGameMap.GetFloorHeightAt(mTransform.mPosition);

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

void GameObject::SetOrientation(cxx::angle_t orientation)
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

void GameObject::SetTransform(const EntityTransform& transform)
{
    SetPosition(transform.mPosition);
    SetOrientation(transform.mOrientation);
}

const cxx::aabbox& GameObject::GetMeshLocalBounds() const
{
    static const cxx::aabbox nullBounds;
    if (mMeshObject)
        return mMeshObject->GetLocalBounds();

    return nullBounds;
}

const cxx::aabbox& GameObject::GetMeshWorldBounds() const
{
    static const cxx::aabbox nullBounds;
    if (mMeshObject)
    {
        mMeshObject->ComputeTransformation();
        return mMeshObject->GetWorldBounds();
    }
    return nullBounds;
}

Point2D GameObject::GetTilePosition() const
{
    const glm::vec3 position = GetPosition();
    return MapUtils::ComputeTileFromPosition(position);
}

void GameObject::OnRecycle()
{
    Entity::OnRecycle();

    mController = nullptr;
    mCurrentState = eGameObjectState_None;
    mParentRoom = {};
    mDefinition = nullptr;
    mMeshResourceId = eGameObjectMeshId_Main;

    FreeMesh();
    FreePhysics();

    mIsHighlighted = {};
}

bool GameObject::HasMeshResource(eGameObjectMeshId meshId) const
{
    switch (meshId)
    {
        case eGameObjectMeshId_Main: return mDefinition->mResourceMesh.IsDefined();
        case eGameObjectMeshId_Additional1: return mDefinition->mResourceAdditional1.IsDefined();
        case eGameObjectMeshId_Additional2: return mDefinition->mResourceAdditional2.IsDefined();
        case eGameObjectMeshId_Additional3: return mDefinition->mResourceAdditional3.IsDefined();
        case eGameObjectMeshId_Additional4: return mDefinition->mResourceAdditional4.IsDefined();
    }
    cxx_assert(false);
    return false;
}

bool GameObject::SetMeshResource(eGameObjectMeshId meshId)
{
    if (mMeshResourceId == meshId) 
        return true;

    std::reference_wrapper<ArtResourceDefinition> artResource = mDefinition->mResourceMesh;

    switch (meshId)
    {
        case eGameObjectMeshId_Additional1: 
            artResource = mDefinition->mResourceAdditional1;
        break;
        case eGameObjectMeshId_Additional2: 
            artResource = mDefinition->mResourceAdditional2;
        break;
        case eGameObjectMeshId_Additional3: 
            artResource = mDefinition->mResourceAdditional3;
        break;
        case eGameObjectMeshId_Additional4: 
            artResource = mDefinition->mResourceAdditional4;
        break;
    }

    if (artResource.get().IsDefined())
    {
        mMeshResourceId = meshId;

        ShowMesh(true);
        ConfigureMeshResource(artResource.get());
        return true;
    }
    return false;
}

void GameObject::ConfigureMeshResource(const ArtResourceDefinition& artResource)
{
    InitMesh();

    if (!mMeshObject) 
        return;

    bool isSucess = false;

    if ((artResource.mResourceType == eArtResource_Mesh) ||
        (artResource.mResourceType == eArtResource_TerrainMesh))
    {
        MeshAsset* meshAsset = gMeshAssetManager.GetMesh(artResource.mResourceName);
        mMeshObject->Configure(meshAsset);
        isSucess = true;
    }

    if (artResource.mResourceType == eArtResource_AnimatingMesh)
    {
        MeshAsset* meshAsset = gMeshAssetManager.GetMesh(artResource.mResourceName);

        AnimationParams animParams;
        animParams.mFramesPerSecond = artResource.mAnimationDesc.mFps * 1.0f;
        animParams.mFirstFrame = 0;
        animParams.mLastFrame = (artResource.mAnimationDesc.mFrames > 0) ? (artResource.mAnimationDesc.mFrames - 1) : 0;
        if (!artResource.mDoesntLoop)
        {
            switch (meshAsset->GetAnimFrameFactorFunc())
            {
                case MeshAsset::eAnimFrameFactorFunc::Clamp: 
                    animParams.mLoopMode = eAnimationLoopMode_PingPong;
                break;
                case MeshAsset::eAnimFrameFactorFunc::Wrap: 
                    animParams.mLoopMode = eAnimationLoopMode_Repeat;
                break;
                default:
                    cxx_assert(false);
                    animParams.mLoopMode = eAnimationLoopMode_Repeat;
                break;
            }
        }
        mMeshObject->Configure(meshAsset, animParams);
        mMeshObject->ResetAnimationSpeedFactor();
        // set random progress
        if (artResource.mRandomStartFrame)
        {
            mMeshObject->SetAnimationProgress(Random::GenerateFloat01());
        }
        isSucess = true;
    }

    cxx_assert(isSucess);
}

void GameObject::SetCurrentState(eGameObjectState stateId)
{
    mCurrentState = stateId;
}

bool GameObject::RescaleAnimationDuration(float animDuration)
{
    cxx_assert(animDuration > 0.0f);
    if (mMeshObject && mMeshObject->HasAnimation())
    {
        float origDuration = mMeshObject->GetAnimationDuration();
        if (animDuration > 0.0f)
        {
            float speedFactor = origDuration / animDuration;
            cxx_assert(speedFactor > 0.0f);
            mMeshObject->SetAnimationSpeedFactor(speedFactor);
        }
        return true;
    }
    return false;
}

void GameObject::ResetAnimationDuration()
{
    if (mMeshObject && mMeshObject->HasAnimation())
    {
        mMeshObject->ResetAnimationSpeedFactor();
    }
}

void GameObject::SetHighlighted(bool isHighlighted)
{
    if (isHighlighted == mIsHighlighted)
        return;

    mIsHighlighted = isHighlighted;
    if (mMeshObject)
    {
        mMeshObject->SetHighlighted(isHighlighted);
    }
}

void GameObject::ParentRoomChanged(EntityHandle roomHandle)
{
    if (mParentRoom == roomHandle) 
        return;

    if (roomHandle && !roomHandle.IsRoom())
    {
        cxx_assert(false);
        return;
    }
    mParentRoom = roomHandle;
    if (mController)
    {
        mController->ParentRoomChanged();
    }
}

void GameObject::EnablePhysics(bool isEnabled)
{
    bool wasEnabled = (mPhysicsObject != nullptr);
    if (wasEnabled != isEnabled)
    {
        if (isEnabled)
        {
            InitPhysics();
        }
        else
        {
            FreePhysics();
        }
    }
}

void GameObject::InitPhysics()
{
    if (mPhysicsObject)
        return;

    gPhysics.AttachUser(this);

    mPhysicsObject = gPhysics.GetPhysicsObject(this);
    cxx_assert(mPhysicsObject);
    mPhysicsObject->ClearAngularVelocity();
    mPhysicsObject->ClearLinearVelocity();
}

void GameObject::FreePhysics()
{
    if (mPhysicsObject)
    {
        gPhysics.DetachUser(this);
        mPhysicsObject = nullptr;
    }
}

void GameObject::MarkDeleted()
{
    mEntityFlags.mWasDeleted = true;
}

void GameObject::UpdateLogic(float stepDeltaTime)
{
    if (mController)
    {
        mController->UpdateLogic(stepDeltaTime);
    }
}

void GameObject::UpdatePhysics(float stepDeltaTime)
{
    // process locomotion
    if (mLocomotion.HasGoals())
    {
        Locomotion::VelocitiesResult velocities = mLocomotion.ProcessGoals(stepDeltaTime, GetTransform());
        // notify self
        if (mLocomotion.HasGoals())
        {
            ReceiveMsg(EntityMsg_LocoApplyVelocities{velocities.mLinearVelocity, velocities.mAngularVelocity});
        }
        else
        {
            ReceiveMsg(EntityMsg_LocoClearVelocities{});
        }
    }
}

void GameObject::ReceiveMsg(EntityMsg& msgData)
{
    if (!Exists()) 
        return;

    if (msgData.Is(EntityMsg::eID_LocoApplyVelocities))
    {
        if (mPhysicsObject)
        {
            mPhysicsObject->SetLinearVelocity(msgData.mLocoVelocities.mLinear);
            mPhysicsObject->SetAngularVelocity(msgData.mLocoVelocities.mAngular);
        }
        msgData.SetConsumed();
        return;
    }

    if (msgData.Is(EntityMsg::eID_LocoClearVelocities))
    {
        if (mPhysicsObject)
        {
            mPhysicsObject->ClearLinearVelocity();
            mPhysicsObject->ClearAngularVelocity();
        }
        msgData.SetConsumed();
        return;
    }

    if (msgData.Is(EntityMsg::eID_SyncWithPhysicsTransform))
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
        msgData.SetConsumed();
        return;
    }
}

