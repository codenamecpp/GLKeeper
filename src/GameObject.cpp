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

GameObject::GameObject()
{
    // leave ctor
}

GameObject::~GameObject()
{
    // leave dtor
}

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
    cxx_assert(!WasSpawned());
    cxx_assert(!WasDespawned());
    cxx_assert(!WasDeleted());

    if (WasSpawned()) 
        return;

    mEntityFlags.Include(eEntityFlags_WasSpawned);

    mOwnHandle = gGameObjectManager.FindObject(mInstanceUid);

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
    cxx_assert(WasSpawned());

    if (WasDespawned()) 
        return;

    if (mController)
    {
        mController->DespawnInstance();
    }

    if (Locomotion* locomotion = GetLocomotion())
    {
        locomotion->ClearGoals();
    }

    EnablePhysics(false);
    ShowMesh(false);

    mOwnHandle = {};

    mEntityFlags.Include(eEntityFlags_WasDespawned);
}

void GameObject::InitMesh()
{
    if (!HasComponent<AnimatingMeshComponent>())
    {
        cxx_assert(GetMeshObject() == nullptr);
        AnimatingMeshComponent* component = AddComponent<AnimatingMeshComponent>();
        cxx_assert(GetMeshObject());
    }
}

void GameObject::ShowMesh(bool isEnabled)
{
    if (AnimatingMeshComponent* component = GetComponent<AnimatingMeshComponent>())
    {
        component->SetEnabled(isEnabled);
    }
}

void GameObject::SetPosition(const glm::vec3& position)
{
    mTransform.mPosition = position;
    if (AnimatingMeshObject* meshObject = GetMeshObject())
    {
        meshObject->SetPosition(mTransform.mPosition);
    }

    // sync physics
    if (PhysicsObject* physics = GetPhysics())
    {
        physics->SetTransform(mTransform);
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
    if (AnimatingMeshObject* meshObject = GetMeshObject())
    {
        meshObject->ResetOrientation();
        meshObject->RotateAroundAxis(WorldAxes::Y, mTransform.mOrientation);
    }

    // sync physics
    if (PhysicsObject* physics = GetPhysics())
    {
        physics->SetTransform(mTransform);
    }
}

void GameObject::SetTransform(const EntityTransform& transform)
{
    SetPosition(transform.mPosition);
    SetOrientation(transform.mOrientation);
}

const cxx::aabbox& GameObject::GetMeshLocalBounds() const
{
    if (AnimatingMeshObject* meshObject = GetMeshObject())
    {
        return meshObject->GetLocalBounds();
    }
    static const cxx::aabbox nullBounds;
    return nullBounds;
}

const cxx::aabbox& GameObject::GetMeshWorldBounds() const
{
    if (AnimatingMeshObject* meshObject = GetMeshObject())
    {
        meshObject->ComputeTransformation();
        return meshObject->GetWorldBounds();
    }
    static const cxx::aabbox nullBounds;
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
    mObjectMeshId = eGameObjectMeshId_Main;
}

bool GameObject::HasObjectMesh(eGameObjectMeshId meshId) const
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

bool GameObject::SetObjectMesh(eGameObjectMeshId meshId)
{
    if (mObjectMeshId == meshId) 
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
        mObjectMeshId = meshId;

        ShowMesh(true);
        ConfigureMeshResource(artResource.get());
        return true;
    }
    return false;
}

void GameObject::SetCustomMesh(const std::string& meshName)
{
    mObjectMeshId = eGameObjectMeshId_None;

    MeshAsset* meshAsset = gMeshAssetManager.GetMesh(meshName);
    cxx_assert(meshAsset);

    InitMesh();

    AnimatingMeshObject* meshObject = GetMeshObject();
    cxx_assert(meshObject);

    if (meshAsset->GetAnimFramesCount() > 0)
    {
        AnimationParams animParams;
        animParams.mFirstFrame = 0;
        animParams.mLastFrame = (meshAsset->GetAnimFramesCount() > 0) ? (meshAsset->GetAnimFramesCount() - 1) : 0;
        animParams.mLoopMode = eAnimationLoopMode_Repeat;

        meshObject->Configure(meshAsset, animParams);
        meshObject->ResetAnimationSpeedFactor();
    }
    else
    {
        meshObject->Configure(meshAsset);
    }
}

void GameObject::ConfigureMeshResource(const ArtResourceDefinition& artResource)
{
    InitMesh();

    AnimatingMeshObject* meshObject = GetMeshObject();
    cxx_assert(meshObject);

    bool isSucess = false;

    if ((artResource.mResourceType == eArtResource_Mesh) ||
        (artResource.mResourceType == eArtResource_TerrainMesh))
    {
        MeshAsset* meshAsset = gMeshAssetManager.GetMesh(artResource.mResourceName);
        meshObject->Configure(meshAsset);
        isSucess = true;
    }

    if (artResource.mResourceType == eArtResource_AnimatingMesh)
    {
        MeshAsset* meshAsset = gMeshAssetManager.GetMesh(artResource.mResourceName);

        AnimationParams animParams;
        animParams.mFramesPerSecond = artResource.mAnimationDesc.mFps * 1.0f;
        animParams.mFirstFrame = 0;
        animParams.mLastFrame = (meshAsset->GetAnimFramesCount() > 0) ? (meshAsset->GetAnimFramesCount() - 1) : 0;
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
        meshObject->Configure(meshAsset, animParams);
        meshObject->ResetAnimationSpeedFactor();
        // set random progress
        if (artResource.mRandomStartFrame)
        {
            meshObject->SetAnimationProgress(Random::GenerateFloat01());
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
    AnimatingMeshObject* meshObject = GetMeshObject();
    if (meshObject && meshObject->HasAnimation())
    {
        float origDuration = meshObject->GetAnimationDuration();
        if (animDuration > 0.0f)
        {
            float speedFactor = origDuration / animDuration;
            cxx_assert(speedFactor > 0.0f);
            meshObject->SetAnimationSpeedFactor(speedFactor);
        }
        return true;
    }
    return false;
}

void GameObject::ResetAnimationDuration()
{
    AnimatingMeshObject* meshObject = GetMeshObject();
    if (meshObject && meshObject->HasAnimation())
    {
        meshObject->ResetAnimationSpeedFactor();
    }
}

void GameObject::SetHighlighted(bool isHighlighted)
{
    if (IsHighlighted() == isHighlighted)
        return;

    mEntityFlags.Set(eEntityFlags_IsHighlighted, isHighlighted);
    if (AnimatingMeshObject* meshObject = GetMeshObject())
    {
        meshObject->SetHighlighted(isHighlighted);
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
    if (PhysicsComponent* component = GetComponent<PhysicsComponent>())
    {
        component->SetEnabled(isEnabled);
    }
}

void GameObject::InitPhysics()
{
    if (HasComponent<PhysicsComponent>())
    {
        cxx_assert(GetPhysics());
        return;
    }

    PhysicsComponent* component = AddComponent<PhysicsComponent>();
    cxx_assert(component->mPhysicsObject);

    if (PhysicsObject* physics = GetPhysics())
    {
        physics->ClearAngularVelocity();
        physics->ClearLinearVelocity();
    }
}

void GameObject::InitLocomotion()
{
    if (HasComponent<LocomotionComponent>())
    {
        cxx_assert(GetLocomotion());
        return;
    }

    LocomotionComponent* component = AddComponent<LocomotionComponent>();
    cxx_assert(component);

    if (Locomotion* locomotion = GetLocomotion())
    {
        locomotion->ResetToDefaults();
        locomotion->ClearGoals();
        locomotion->SetArriveSpeed(mDefinition->mSpeed);
    }
}

void GameObject::MarkDeleted()
{
    mEntityFlags.Include(eEntityFlags_WasDeleted);
}

void GameObject::UpdateLogic(float stepDeltaTime)
{
    if (mController)
    {
        mController->UpdateLogic(stepDeltaTime);
    }
}

void GameObject::UpdateFrame(float deltaTime)
{
    if (mController)
    {
        mController->UpdateFrame(deltaTime);
    }
}

void GameObject::UpdatePhysics(float stepDeltaTime)
{
    // process locomotion
    if (Locomotion* locomotion = GetLocomotion())
    {
        if (locomotion->HasGoals())
        {
            Locomotion::VelocitiesResult velocities = locomotion->ProcessGoals(stepDeltaTime, GetTransform());
            // notify self
            if (locomotion->HasGoals())
            {
                ReceiveMsg(EntityMsg_LocoApplyVelocities{velocities.mLinearVelocity, velocities.mAngularVelocity});
            }
            else
            {
                ReceiveMsg(EntityMsg_LocoClearVelocities{});
            }
        }
    }
}

void GameObject::ReceiveMsg(EntityMsg& msgData)
{
    if (!Exists()) 
        return;

    if (msgData.Is(EntityMsg::eID_LocoApplyVelocities))
    {
        if (PhysicsObject* physics = GetPhysics())
        {
            physics->SetLinearVelocity(msgData.mLocoVelocities.mLinear);
            physics->SetAngularVelocity(msgData.mLocoVelocities.mAngular);
        }
        msgData.SetConsumed();
        return;
    }

    if (msgData.Is(EntityMsg::eID_LocoClearVelocities))
    {
        if (PhysicsObject* physics = GetPhysics())
        {
            physics->ClearLinearVelocity();
            physics->ClearAngularVelocity();
        }
        msgData.SetConsumed();
        return;
    }

    if (msgData.Is(EntityMsg::eID_SyncWithPhysicsTransform))
    {
        if (PhysicsObject* physics = GetPhysics())
        {
            mTransform = physics->GetTransform();
            if (AnimatingMeshObject* meshObject = GetMeshObject())
            {
                meshObject->SetPosition(mTransform.mPosition);
                meshObject->ResetOrientation();
                meshObject->RotateAroundAxis(WorldAxes::Y, mTransform.mOrientation);
            }
        }
        msgData.SetConsumed();
        return;
    }
}
