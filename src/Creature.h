#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureDefs.h"
#include "GameDefs.h"
#include "ScenarioDefs.h"
#include "GameSessionAware.h"
#include "Entity.h"
#include "PhysicsDefs.h"
#include "AnimatingMeshObject.h"
#include "Locomotion.h"
#include "Animator.h"
#include "CreatureActivity.h"
#include "CreatureActivityUtils.h"

//////////////////////////////////////////////////////////////////////////

class Creature final: public Entity
{
    friend class CreatureController;

public:

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

    MapPoint2D GetTilePosition() const;

    // scene object shortcuts
    const cxx::aabbox& GetMeshLocalBounds() const;
    const cxx::aabbox& GetMeshWorldBounds() const;

    // definition shortcuts
    inline CreatureDefinition* GetDefinition() const { return mDefinition; }
    inline CreatureTypeId GetCreatureTypeId() const 
    { 
        return mDefinition->mCreatureTypeId; 
    }

    // creature activities
    inline bool HasActivity() const { return !!mCurrentActivity; }
    inline eCreatureActivity GetCurrentActivityType() const
    {
        return mCurrentActivity ? CreatureActivityUtils::GetType(*mCurrentActivity) : eCreatureActivity_None;
    }
    inline eCreatureActivity GetRequestActivityType() const
    {
        return mRequestActivity ? CreatureActivityUtils::GetType(*mRequestActivity) : eCreatureActivity_None; 
    }
    inline CreatureActivity* GetCurrentActivity() const { return mCurrentActivity.get(); }
    inline CreatureActivity* GetRequestActivity() const { return mRequestActivity.get(); }

    // accessing primary components
    inline AnimatingMeshObject* GetMeshObject() const { return mMeshObject.get(); }

    inline Locomotion& GetLocomotion() { return mLocomotion; }

    inline Animator& GetAnimator() { return mAnimator; }

    // override Entity
    void Notify(const EntityNotification& notificationData) override;

private:
    // enable or disable primary components
    void EnableMeshObject(bool isEnabled);
    void EnablePhysics(bool isEnabled);

    //////////////////////////////////////////////////////////////////////////
    
    // activity

    template<typename TActivity, typename ... TArgs>
    inline void RequestActivity(TArgs&&... args)
    {
        mRequestActivity = CreatureActivityUtils::Construct<TActivity>(std::forward<TArgs>(args)...);
    }

    void ClearCurrentActivity();
    void ClearRequestActivity();
    void SwitchToRequestActivity();
    void CancelCurrentActivity();

    //////////////////////////////////////////////////////////////////////////

private:
    ePlayerID mOwnerID = ePlayerID_Null;

    CreatureDefinition* mDefinition = nullptr; // never changes
    CreatureController* mController = nullptr;
    PhysicsObject* mPhysicsObject = nullptr; // optional
    Locomotion mLocomotion;

    cxx::uniqueptr<AnimatingMeshObject> mMeshObject;
    Animator mAnimator;

    cxx::uniqueptr<CreatureActivity> mCurrentActivity;
    cxx::uniqueptr<CreatureActivity> mRequestActivity;
};

//////////////////////////////////////////////////////////////////////////