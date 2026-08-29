#include "stdafx.h"
#include "Creature.h"
#include "CreatureController.h"
#include "CreatureManager.h"
#include "Scene.h"
#include "MeshAssetManager.h"
#include "PhysicsObject.h"
#include "GameWorld.h"
#include "Physics.h"
#include "GameMap.h"
#include "CreatureTaskManager.h"
#include "CreatureTask.h"

Creature::Creature()
{
}

Creature::~Creature()
{
    cxx_assert(mAssignedTask == nullptr);
}

void Creature::ConfigureInstance(EntityUid instanceUid, CreatureController* controller, CreatureDefinition* definition, ePlayerID owner)
{
    mInstanceUid = instanceUid;

    mOwnerId = owner;
    cxx_assert(mOwnerId != ePlayerID_Null);

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
    cxx_assert(!mEntityFlags.mWasSpawned);
    cxx_assert(!mEntityFlags.mWasDespawned);
    cxx_assert(!mEntityFlags.mWasDeleted);

    if (mEntityFlags.mWasSpawned) 
        return;

    mEntityFlags.mWasSpawned = true;

    mOwnHandle = gCreatureManager.FindCreature(mInstanceUid);

    InitMesh();
    EnableMesh(true);
    
    // configure locomotion
    {
        mLocomotion.ResetToDefaults();
        mLocomotion.SetArriveSpeed(mDefinition->mSpeed);
    }

    InitPhysics();
    EnablePhysics(true);

    if (mController)
    {
        mController->SpawnInstance();
    }

    if (mCurrState == nullptr)
    {
        SelectState();
    }
}

void Creature::DespawnInstance()
{
    cxx_assert(mEntityFlags.mWasSpawned);

    if (mEntityFlags.mWasDespawned)
        return;

    ChangeState(eCreatureState_None);
    mCurrState.reset();
    mPrevState.reset();

    if (mController)
    {
        mController->DespawnInstance();
    }

    mOwnerId = ePlayerID_Null;
    mOwnHandle = {};

    mLocomotion.ResetToDefaults();
    mLocomotion.ClearGoals();

    EnablePhysics(false);
    EnableMesh(false);

    UnassignCurrentTask();

    mEntityFlags.mWasDespawned = true;
}

void Creature::UpdateLogic(float stepDeltaTime)
{
    // advance current state
    if (mCurrState)
    {
        mCurrState->UpdateLogic(stepDeltaTime);
    }
  
    if (mCurrState == nullptr)
    {
        SelectState();
    }

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
            ReceiveMsg(EntityMsg_LocoApplyVelocities{velocities.mLinearVelocity, velocities.mAngularVelocity});
        }
        else
        {
            ReceiveMsg(EntityMsg_LocoClearVelocities{});
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

Point2D Creature::GetTilePosition() const
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

long Creature::GetMoneyCarried() const
{
    long resultAmount = 0;
    if (const MoneyComponent* moneyComponent = GetComponent<MoneyComponent>())
    {
        resultAmount = moneyComponent->mAmount;
    }
    return resultAmount;
}

bool Creature::CanCarryMoreMoney() const
{
    bool canCarryMore = false;
    if (const MoneyComponent* moneyComponent = GetComponent<MoneyComponent>())
    {
        canCarryMore = moneyComponent->mAmount < moneyComponent->mCapacity;
    }
    return canCarryMore;
}

long Creature::ReceiveMoney(long moneyAmount, long& leftoverAmount)
{
    long currentAmount = 0;
    long previousAmount = 0;
    cxx_assert(moneyAmount > 0);
    if (MoneyComponent* moneyComponent = GetComponent<MoneyComponent>())
    {
        previousAmount = moneyComponent->mAmount;
        if (moneyAmount > 0)
        {
            cxx_assert(moneyComponent->mCapacity > 0);
            currentAmount = std::clamp<long>(previousAmount + moneyAmount, 0, moneyComponent->mCapacity);
            moneyComponent->mAmount = currentAmount;
        }
        else
        {
            currentAmount = previousAmount;
        }
    }
    leftoverAmount = moneyAmount - (currentAmount - previousAmount);
    return currentAmount;
}

long Creature::WithdrawMoney(long moneyAmount)
{
    long currentAmount = 0;
    long previousAmount = 0;
    cxx_assert(moneyAmount > 0);
    if (MoneyComponent* moneyComponent = GetComponent<MoneyComponent>())
    {
        previousAmount = moneyComponent->mAmount;
        currentAmount = std::clamp<long>(previousAmount - moneyAmount, 0, moneyComponent->mCapacity);
        moneyComponent->mAmount = currentAmount;
    }
    return currentAmount;
}

void Creature::SetHighlighted(bool isHighlighted)
{
    if (mStateFlags.mIsHighlighted == isHighlighted)
        return;

    mStateFlags.mIsHighlighted = isHighlighted;
    if (mMeshObject)
    {
        mMeshObject->SetHighlighted(isHighlighted);
    }
}

void Creature::MarkDeleted()
{
    mEntityFlags.mWasDeleted = true;
}

void Creature::OnRecycle()
{
    Entity::OnRecycle();

    mController = nullptr;
    mDefinition = nullptr;

    cxx_assert(mPrevState == nullptr);
    mPrevState.reset();

    cxx_assert(mCurrState == nullptr);
    mCurrState.reset();

    cxx_assert(mAssignedTask == nullptr);
    mAssignedTask = nullptr;

    mLastAssignedJob = {};

    mLocomotion.ResetToDefaults();
    mLocomotion.ClearGoals();

    FreePhysics();
    FreeMesh();

    mStateFlags = {};
}

void Creature::InitMesh()
{
    if (mMeshObject)
        return;

    // init mesh
    mMeshObject = gScene.CreateAnimatingMesh();
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
        mMeshObject->SetHighlighted(IsHighlighted());
        mMeshObject->SetOwnerEntity(GetOwnHandle());
        mMeshObject->SetObjectActive(true);
    }
    mAnimator.Configure(mMeshObject.get());
}

void Creature::FreeMesh()
{
    mMeshObject.reset();
    mAnimator.Clear();
}

void Creature::EnableMesh(bool isEnabled)
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

void Creature::InitPhysics()
{
    if (mPhysicsObject)
        return;

    gPhysics.AttachUser(this);

    mPhysicsObject = gPhysics.GetPhysicsObject(this);
    cxx_assert(mPhysicsObject);
    mPhysicsObject->ClearAngularVelocity();
    mPhysicsObject->ClearLinearVelocity();
}

void Creature::FreePhysics()
{
    if (mPhysicsObject)
    {
        gPhysics.DetachUser(this);
        mPhysicsObject = nullptr;
    }
}

void Creature::EnablePhysics(bool isEnabled)
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

void Creature::ReceiveMsg(EntityMsg& msgData)
{
    if (!Exists())
        return;

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

    if (msgData.Is(EntityMsg::eID_LocoApplyVelocities))
    {
        if (mPhysicsObject)
        {
            mPhysicsObject->SetLinearVelocity(msgData.mLocoVelocities.mLinear);
            mPhysicsObject->SetAngularVelocity(msgData.mLocoVelocities.mAngular);
        }
        msgData.SetConsumed();
    }

    if (msgData.Is(EntityMsg::eID_LocoClearVelocities))
    {
        if (mPhysicsObject)
        {
            mPhysicsObject->ClearLinearVelocity();
            mPhysicsObject->ClearAngularVelocity();
        }
        msgData.SetConsumed();
    }

    if (mCurrState)
    {
        mCurrState->ReceiveMsg(msgData);
    }

    if (mController)
    {
        mController->HandleMessage(msgData);
    }
}

ePassabilityType Creature::GetPassabilityType() const
{
    ePassabilityType passabilityType = ePassabilityType_Land;
    if (mDefinition)
    {
        if (mDefinition->mCanWalkOnWater || mDefinition->mCanWalkOnLava)
        {
            // assuming that there is no such case when creature can walk on lava but cannot walk on water
            passabilityType = mDefinition->mCanWalkOnLava ? 
                ePassabilityType_Land_Any :  
                ePassabilityType_Land_Water;
        }
    }
    return passabilityType;
}

void Creature::UnassignCurrentTask()
{
    mAssignedTask.reset();
}

void Creature::AssignTask(CreatureTaskPtr&& creatureTask)
{
    cxx_assert(creatureTask);
    if (creatureTask == nullptr)
        return;

    cxx_assert(mAssignedTask != creatureTask);
    if (mAssignedTask == creatureTask)
        return;

    UnassignCurrentTask();

    mAssignedTask = std::move(creatureTask);
    mLastAssignedJob = mAssignedTask->GetJobType();
}

void Creature::ChangeState(CreatureStatePtr&& creatureState)
{
    if ((mCurrState == nullptr) && (creatureState == nullptr))
        return;

    eCreatureState prevState = eCreatureState_None;
    eCreatureState nextState = eCreatureState_None;

    if (creatureState)
    {
        nextState = creatureState->GetStateId();
    }

    // shutdown current state
    if (mCurrState)
    {
        prevState = mCurrState->GetStateId();
        cxx_assert(prevState != nextState);

        mPrevState.reset();
        mPrevState.swap(mCurrState);

        if (mPrevState)
        {
            mPrevState->LeaveState(nextState);
        }
    }

    // sanity check
    cxx_assert(mCurrState == nullptr);

    if (mCurrState)
        return;

    mCurrState.swap(creatureState);
    if (mCurrState)
    {
        mCurrState->Configure(this);
        mCurrState->EnterState(prevState);
    }
}

void Creature::ChangeState(eCreatureState stateId)
{
    CreatureStatePtr newState;
    if (stateId != eCreatureState_None)
    {
        newState = gCreatureManager.CreateState(this, stateId);
    }
    ChangeState(std::move(newState));
}

void Creature::SelectState()
{
    if (mCurrState)
        return;

    if (mAssignedTask)
    {
        ChangeState(eCreatureState_Working);
        return;
    }

    // default
    ChangeState(eCreatureState_Idle);
}

bool Creature::SelectBestTask()
{
    // todo: refactore

    CreatureTaskPtr assignTask;

    const CreatureDefinition* creatureDefs = GetDefinition();
    if (creatureDefs->mIsWorker)
    {
        // search for worker tasks

        if (assignTask == nullptr)
        {
            assignTask = gCreatureTaskManager.GetClaimTerritoryTask(this);
        }

        if ((assignTask == nullptr) && !CanCarryMoreMoney())
        {
            assignTask = gCreatureTaskManager.GetCarryGoldToTreasuryTask(this);
        }

        if (assignTask == nullptr)
        {
            assignTask = gCreatureTaskManager.GetDiggingTask(this);
        }

        if (assignTask == nullptr)
        {
            assignTask = gCreatureTaskManager.GetMiningTask(this);
        }

        if (assignTask == nullptr)
        {
            assignTask = gCreatureTaskManager.GetReinforceWallTask(this);
        }
    }

    if (assignTask == nullptr)
    {
        assignTask = gCreatureTaskManager.GetWanderTask(this);
    }

    if (assignTask)
    {
        AssignTask(std::move(assignTask));
        return true;
    }

    return false;
}

bool Creature::SelectTaskForJob(eCreatureJob jobType)
{
    if (jobType == eCreatureJob_None)
        return false;

    // todo: refactore

    CreatureTaskPtr assignTask;

    const CreatureDefinition* creatureDefs = GetDefinition();
    if (creatureDefs->mIsWorker)
    {
        switch (jobType)
        {
            case eCreatureJob_Claim:
            {
                if (assignTask == nullptr)
                {
                    assignTask = gCreatureTaskManager.GetClaimTerritoryTask(this);
                }
            }
            // fallthrough ->
            case eCreatureJob_Mine:
            case eCreatureJob_CarryGoldToTreasury:
            {
                if ((assignTask == nullptr) && (GetMoneyCarried() > 0))
                {
                    assignTask = gCreatureTaskManager.GetCarryGoldToTreasuryTask(this);
                }

                if (assignTask == nullptr)
                {
                    assignTask = gCreatureTaskManager.GetMiningTask(this);
                }
            }
            // fallthrough ->
            case eCreatureJob_Dig:
            {
                if (assignTask == nullptr)
                {
                    assignTask = gCreatureTaskManager.GetDiggingTask(this);
                }
            }
            // fallthrough ->
            case eCreatureJob_ReinforceWall:
            {
                if (assignTask == nullptr)
                {
                    assignTask = gCreatureTaskManager.GetReinforceWallTask(this);
                }
            }
            // fallthrough ->
            default: break;
        }
    }

    if (assignTask)
    {
        AssignTask(std::move(assignTask));
        return true;
    }

    return false;
}

bool Creature::CanPickUp() const
{
    if (!ExistsOnMap())
        return false;

    // todo: check state / flags

    return true;
}

bool Creature::PickUp()
{
    if (!CanPickUp())
        return false;

    mStateFlags.mInHand = true;
    SetEntityUnplaced(true);

    ChangeState(eCreatureState_InHand);
    return true;
}

bool Creature::DropOn(const glm::vec2& position)
{
    if (!IsPickedUp())
        return false;

    SetEntityUnplaced(false);

    mStateFlags.mInHand = false;

    float dropHeight = MAP_FLOOR_LEVEL + MAP_BLOCK_HEIGHT;

    SetPosition(glm::vec3{position.x, dropHeight, position.y});

    ChangeState(eCreatureState_Idle);
    return true;
}

