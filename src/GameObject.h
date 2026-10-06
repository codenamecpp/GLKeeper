#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "GameObjectDefs.h"
#include "ScenarioDefs.h"
#include "AnimatingMeshObject.h"
#include "Entity.h"
#include "PhysicsDefs.h"
#include "Locomotion.h"

//////////////////////////////////////////////////////////////////////////

// Defines generic object in dungeon such as room furniture, pillar, crate, gold etc

class GameObject final: public Entity
{
public:
    GameObject();
    ~GameObject();

    //////////////////////////////////////////////////////////////////////////
    // lifecycle
    //////////////////////////////////////////////////////////////////////////

    // performs early initialization of the object, before it is spawned
    // called by GameObjectManager during CreateObject()
    void ConfigureInstance(EntityUid instanceUid, GameObjectController* objectController, GameObjectDefinition* objectDef);

    // preloads resources, adds the object to the game world and activates it
    // called by GameObjectManager during ActivateObject()
    void SpawnInstance();

    // deactivates the object and releases its internal resources
    // called by GameObjectManager during DeleteObject()
    void DespawnInstance();

    // think
    void UpdateLogic(float stepDeltaTime);

    // variable delta update
    void UpdateFrame(float deltaTime);

    // fixed update, physics related
    void UpdatePhysics(float stepDeltaTime);

    // mark object as pending deletion
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
    void SetTransform(const EntityTransform& transform);

    Point2D GetTilePosition() const;

    //////////////////////////////////////////////////////////////////////////
    // description
    //////////////////////////////////////////////////////////////////////////

    inline GameObjectDefinition* GetDefinition() const { return mDefinition; }

    // definition shortcuts
    inline GameObjectClassId GetClassId() const { return mDefinition->mObjectClass; }
    inline eGameObjectCategory GetCategory() const { return mDefinition->mObjectCategory; }

    inline EntityHandle GetParentRoom() const { return mParentRoom; }

    //////////////////////////////////////////////////////////////////////////

    // component shortcuts

    inline AnimatingMeshObject* GetMeshObject() const 
    {
        const AnimatingMeshComponent* component = GetComponent<AnimatingMeshComponent>();
        return component ? component->mMeshObject.get() : nullptr;
    }

    inline PhysicsObject* GetPhysics() const
    {
        const PhysicsComponent* component = GetComponent<PhysicsComponent>();
        return component ? component->mPhysicsObject.get() : nullptr;
    }

    inline Locomotion* GetLocomotion() const
    {
        const LocomotionComponent* component = GetComponent<LocomotionComponent>();
        return component ? component->mLoco.get() : nullptr;
    }

    //////////////////////////////////////////////////////////////////////////

    // scene object shortcuts
    const cxx::aabbox& GetMeshLocalBounds() const;
    const cxx::aabbox& GetMeshWorldBounds() const;

    //////////////////////////////////////////////////////////////////////////

    // highlight control
    void SetHighlighted(bool isHighlighted);

    //////////////////////////////////////////////////////////////////////////
    // notifications
    //////////////////////////////////////////////////////////////////////////

    void ParentRoomChanged(EntityHandle roomHandle);

    //////////////////////////////////////////////////////////////////////////

    // override Entity
    void ReceiveMsg(EntityMsg& msgData) override;

public:

    //////////////////////////////////////////////////////////////////////////
    // part of the internal GameObject Controllers API, not intended for public use
    //////////////////////////////////////////////////////////////////////////

    void InitLocomotion();

    // physics control
    void InitPhysics();
    void EnablePhysics(bool isEnabled);

    // mesh control
    void ShowMesh(bool isEnabled);
    bool HasObjectMesh(eGameObjectMeshId meshId) const;
    bool SetObjectMesh(eGameObjectMeshId meshId);
    void SetCustomMesh(const std::string& meshName);

    // mesh animation params
    bool RescaleAnimationDuration(float animDuration);
    void ResetAnimationDuration();

    // current state
    void SetCurrentState(eGameObjectState stateId);
    inline eGameObjectState GetCurrentState() const { return mCurrentState; }

private:
    void InitMesh();
    void ConfigureMeshResource(const ArtResourceDefinition& artResource);

private:
    GameObjectDefinition* mDefinition = nullptr; // never changes
    // the room where the object is stored, optional
    EntityHandle mParentRoom;
    eGameObjectState mCurrentState = eGameObjectState_None;
    GameObjectController* mController = nullptr; // optional
    eGameObjectMeshId mObjectMeshId = eGameObjectMeshId_Main;
};

//////////////////////////////////////////////////////////////////////////