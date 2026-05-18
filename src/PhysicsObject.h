#pragma once

//////////////////////////////////////////////////////////////////////////

#include "PhysicsDefs.h"

//////////////////////////////////////////////////////////////////////////

class PhysicsObject final: public cxx::noncopyable
{
    friend class Physics;

public:
    PhysicsObject() = default;

    void Configure(Entity* entity, const EntityTransform& entityTransform);

    // pool
    void OnRecycle();

    // overrides current transform
    void SetTransform(const EntityTransform& entityTransform);

    void SetLinearVelocity(const glm::vec2& velocity);
    void AddLinearVelocity(const glm::vec2& velocity);
    void ClearLinearVelocity();
    const glm::vec2& GetLinearVelocity() const { return mLinearVelocity; }

    bool HasLinearVelocity() const;

    // linear speed and linear speed squared shortcuts
    inline float GetLinearSpeed () const { return glm::length(mLinearVelocity);  }
    inline float GetLinearSpeed2() const { return glm::length2(mLinearVelocity); }

    void SetAngularVelocity(float velocity);
    void AddAngularVelocity(float velocity);
    void ClearAngularVelocity();
    float GetAngularVelocity() const { return mAngularVelocity; }

    bool HasAngularVelocity() const;

    inline const EntityTransform& GetTransform() const { return mCurrTransform; }

    // shortcuts
    inline const glm::vec3& GetPosition() const { return mCurrTransform.mPosition; }
    inline glm::vec2 GetPosition2d() const
    {
        return { mCurrTransform.mPosition.x, mCurrTransform.mPosition.z };
    }
    inline cxx::angle_t GetOrientation() const 
    { 
        return mCurrTransform.mOrientation; 
    }

private:
    Entity* mEntity = nullptr; // bridge to entity

    // the game entities are basically 2D objects in a 3D world, so they only move horizontally
    glm::vec2 mLinearVelocity {};

    // likewise, rotation is only relevant around the world Y
    float mAngularVelocity {};

    EntityTransform mCurrTransform; // interpolated prev/next
    EntityTransform mPrevTransform;
    EntityTransform mNextTransform;
};
