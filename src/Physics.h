#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameObjectDefs.h"
#include "PhysicsObject.h"

//////////////////////////////////////////////////////////////////////////

class Physics final: public cxx::noncopyable
{
public:
    // start / finish game session
    bool LoadScenario(const ScenarioDefinition& scenarioDef);
    void EnterWorld();
    void ClearWorld();

    void UpdateFrame(float deltaTime);
    void UpdatePhysics(float stepDeltaTime);

    // attach entity to physics world
    // only a single physics object per entity allowed

    PhysicsObjectPtr CreatePhysicsObject(Entity* entity);

private:
    void InterpolationStep(PhysicsObject* object, float t);
    void SimulationStep(PhysicsObject* object);
    void ResetVelocities(PhysicsObject* object);

    void Unregister(PhysicsObject* object);

private:
    float mSimulationStepDelta = 0.0f;
    float mInterpolationTime = 0.0f;

    //////////////////////////////////////////////////////////////////////////

    using EntityEntry = std::pair<Entity*, PhysicsObject*>;

    //////////////////////////////////////////////////////////////////////////

    std::vector<EntityEntry> mEntities;
    std::vector<PhysicsObject*> mObjects;
    std::vector<PhysicsObject*> mInterpolateTransforms;
};

//////////////////////////////////////////////////////////////////////////

extern Physics gPhysics;

//////////////////////////////////////////////////////////////////////////