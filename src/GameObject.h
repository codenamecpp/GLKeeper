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

    // accessing primary components
    inline AnimatingMeshObject* GetMeshObject() const 
    { 
        return mMeshObject.get(); 
    }
    inline PhysicsObject* GetPhysics() const { return mPhysicsObject; }

    inline Locomotion& GetLocomotion() { return mLocomotion; }

    //////////////////////////////////////////////////////////////////////////

    // scene object shortcuts
    const cxx::aabbox& GetMeshLocalBounds() const;
    const cxx::aabbox& GetMeshWorldBounds() const;

    //////////////////////////////////////////////////////////////////////////

    // highlight control
    void SetHighlighted(bool isHighlighted);
    bool IsHighlighted() const { return mIsHighlighted; }

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

    // physics control
    void InitPhysics();
    void FreePhysics();
    void EnablePhysics(bool isEnabled);

    // mesh control
    void InitMesh();
    void FreeMesh();
    void ShowMesh(bool isEnabled);
    bool HasMeshResource(eGameObjectMeshId meshId) const;
    bool SetMeshResource(eGameObjectMeshId meshId);

    // mesh animation params
    bool RescaleAnimationDuration(float animDuration);
    void ResetAnimationDuration();

    // current state
    void SetCurrentState(eGameObjectState stateId);
    inline eGameObjectState GetCurrentState() const { return mCurrentState; }

    //////////////////////////////////////////////////////////////////////////

private:
    void ConfigureMeshResource(const ArtResourceDefinition& artResource);

private:
    GameObjectDefinition* mDefinition = nullptr; // never changes
    // the room where the object is stored, optional
    EntityHandle mParentRoom;
    eGameObjectState mCurrentState = eGameObjectState_None;
    GameObjectController* mController = nullptr; // optional
    PhysicsObject* mPhysicsObject = nullptr; // optional
    Locomotion mLocomotion;
    cxx::uniqueptr<AnimatingMeshObject> mMeshObject; // optional
    eGameObjectMeshId mMeshResourceId = eGameObjectMeshId_Main;

    bool mIsHighlighted {};
};

//////////////////////////////////////////////////////////////////////////