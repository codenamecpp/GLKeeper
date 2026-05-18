#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObjectDefs.h"
#include "PhysicsObject.h"
#include "GameSessionAware.h"

//////////////////////////////////////////////////////////////////////////

class Physics final: private GameSessionAware
{
public:
    // start / finish game session
    bool LoadScenario(const ScenarioDefinition& scenarioDef);
    void EnterWorld();
    void ClearWorld();

    void UpdateFrame(float deltaTime);
    void UpdatePhysics(float stepDeltaTime);

    void AttachUser(Entity* entity);
    void DetachUser(Entity* entity);

    PhysicsObject* GetPhysicsObject(Entity* entity) const;

private:
    // factory
    cxx::uniqueptr<PhysicsObject> CreatePhysicsObject();

    void InterpolationStep(PhysicsObject* object, float t);
    void SimulationStep(PhysicsObject* object);
    void ResetVelocities(PhysicsObject* object);

private:
    float mSimulationStepDelta = 0.0f;
    float mInterpolationTime = 0.0f;

    //////////////////////////////////////////////////////////////////////////

    using EntityEntry = std::pair<Entity*, cxx::uniqueptr<PhysicsObject>>;

    //////////////////////////////////////////////////////////////////////////

    std::vector<EntityEntry> mEntities;
    std::vector<PhysicsObject*> mObjects;
    std::vector<PhysicsObject*> mInterpolateTransforms;
};