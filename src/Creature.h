#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureDefs.h"
#include "GameDefs.h"
#include "ScenarioDefs.h"
#include "Entity.h"
#include "PhysicsDefs.h"
#include "AnimatingMeshObject.h"
#include "Locomotion.h"
#include "Animator.h"
#include "CreatureState.h"
#include "CreatureComponents.h"

//////////////////////////////////////////////////////////////////////////

class Creature final: public Entity
{
    friend class CreatureController;

    // states
    friend class CreatureState_Idle;
    friend class CreatureState_Working;
    friend class CreatureState_InHand;

public:
    Creature();
    ~Creature();

    //////////////////////////////////////////////////////////////////////////
    // lifecycle
    //////////////////////////////////////////////////////////////////////////

    // performs early initialization of the creature, before it is spawned
    // called by CreatureManager during CreateCreature()
    void ConfigureInstance(EntityUid instanceUid, CreatureController* controller, CreatureDefinition* definition, ePlayerID owner);

    // preloads resources, adds the creature to the game world and activates it
    // called by CreatureManager during ActivateCreature()
    void SpawnInstance();

    // deactivates the creature and releases its internal resources
    // called by CreatureManager during DeleteCreature()
    void DespawnInstance();

    // variable update, presentation
    void UpdateFrame(float deltaTime);

    // fixed update, think
    void UpdateLogic(float stepDeltaTime);

    // fixed update, physics related
    void UpdatePhysics(float stepDeltaTime);

    // mark creature as pending deletion
    void MarkDeleted();

    // pool
    void OnRecycle();

    //////////////////////////////////////////////////////////////////////////
    // position and orientation
    //////////////////////////////////////////////////////////////////////////

    void SetPosition(const glm::vec3& position);
    void SetPosition(const glm::vec2& position);
    void SnapPositionToFloor(bool withRespectToMeshBounds = false);
    void SetOrientation(cxx::angle_t orientation);

    Point2D GetTilePosition() const;

    // scene object shortcuts
    const cxx::aabbox& GetMeshLocalBounds() const;
    const cxx::aabbox& GetMeshWorldBounds() const;

    // definition shortcuts
    inline CreatureDefinition* GetDefinition() const { return mDefinition; }
    inline CreatureTypeId GetCreatureTypeId() const 
    { 
        return mDefinition->mCreatureTypeId; 
    }
    inline bool IsCreature(CreatureTypeId typeId) const { return typeId == GetCreatureTypeId(); }

    // get current passability
    // depending on creature's abilities its passability type may change
    ePassabilityType GetPassabilityType() const;

    //////////////////////////////////////////////////////////////////////////

    // get current held amount of gold
    long GetMoneyCarried() const;
    bool CanCarryMoreMoney() const;
    // give gold to creature, returns current held amount of gold
    long ReceiveMoney(long moneyAmount, long& leftoverAmount);
    // take gold from creature, returns current held amount of gold
    long WithdrawMoney(long moneyAmount);

    //////////////////////////////////////////////////////////////////////////

    // highlight control
    bool IsHighlighted() const { return mStateFlags.mIsHighlighted; }

    void SetHighlighted(bool isHighlighted);

    //////////////////////////////////////////////////////////////////////////

    // pick control

    // during pickup/dropon only basic rules are checked (state-dependent)
    // ignores special rules like creature ownership or tile availability
    bool CanPickUp() const;
    bool IsPickedUp() const { return mStateFlags.mInHand; }
    bool PickUp();
    bool DropOn(const glm::vec2& position);

    //////////////////////////////////////////////////////////////////////////

    // get current / previous state
    inline bool InState(eCreatureState stateId) const { return GetStateId() == stateId; }
    inline eCreatureState GetStateId() const
    {
        return mCurrState ? mCurrState->GetStateId() : eCreatureState_None;
    }

    inline bool WasInState(eCreatureState stateId) const { return GetPreviousStateId() == stateId; }
    inline eCreatureState GetPreviousStateId() const
    {
        return mPrevState ? mPrevState->GetStateId() : eCreatureState_None;
    }

    // accessing assigned task
    inline CreatureTask* GetAssignedTask() const 
    { 
        return mAssignedTask.get(); 
    }

    inline eCreatureJob GetLastAssignedJob() const { return mLastAssignedJob; }

    // accessing primary components
    inline AnimatingMeshObject* GetMeshObject() const 
    { 
        return mMeshObject.get(); 
    }
    inline Locomotion& GetLocomotion() { return mLocomotion; }
    inline Animator& GetAnimator() { return mAnimator; }

    // override Entity
    void ReceiveMsg(EntityMsg& msgData) override;

private:

    //////////////////////////////////////////////////////////////////////////

    // mesh control
    void InitMesh();
    void FreeMesh();
    void EnableMesh(bool isEnabled);

    //////////////////////////////////////////////////////////////////////////

    // physics control
    void InitPhysics();
    void FreePhysics();
    void EnablePhysics(bool isEnabled);

    //////////////////////////////////////////////////////////////////////////

    // task
    bool SelectBestTask();
    bool SelectTaskForJob(eCreatureJob jobType);
    void AssignTask(CreatureTaskPtr&& creatureTask);
    void UnassignCurrentTask();

    //////////////////////////////////////////////////////////////////////////

    // state
    void SelectState();
    void ChangeState(eCreatureState stateId);
    void ChangeState(CreatureStatePtr&& creatureState);

    //////////////////////////////////////////////////////////////////////////

private:
    CreatureDefinition* mDefinition = nullptr; // never changes
    CreatureController* mController = nullptr;
    PhysicsObject* mPhysicsObject = nullptr; // optional
    Locomotion mLocomotion;

    CreatureTaskPtr mAssignedTask;
    eCreatureJob mLastAssignedJob {};

    cxx::uniqueptr<AnimatingMeshObject> mMeshObject;
    Animator mAnimator;

    CreatureStatePtr mCurrState;
    CreatureStatePtr mPrevState;

    CreatureStateFlags mStateFlags {};
};

//////////////////////////////////////////////////////////////////////////