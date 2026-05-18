#pragma once

//////////////////////////////////////////////////////////////////////////

#include "EntityDefs.h"

//////////////////////////////////////////////////////////////////////////

// simple locomotion controller

//////////////////////////////////////////////////////////////////////////

class Locomotion final
{
public:

    //////////////////////////////////////////////////////////////////////////
    struct VelocitiesResult
    {
        // orientation
        float mAngularVelocity {};

        // movement
        glm::vec2 mLinearVelocity {};
    };
    //////////////////////////////////////////////////////////////////////////

private:

    //////////////////////////////////////////////////////////////////////////
    struct LocomotionGoal
    {
        std::optional<cxx::angle_t> mDesiredOrientation;
        std::optional<glm::vec2> mDesiredPosition;
        // if enabled, automatically updates desired orientation to match movement direction
        bool mAutoOrientation = false;
    };
    //////////////////////////////////////////////////////////////////////////

public:
    Locomotion()
    {
        ResetToDefaults();
    }

    // reset to defaults params
    void ResetToDefaults();

    // set locomotion params
    void SetArriveSpeed(float moveSpeedPerTile);
    void SetOrientSpeed(float radsPerSecond);
    void SetAlignTolerance(float tolerance);
    void SetArrivalTolerance(float tolerance);

    // compute locomotion velocities for next simulation step
    VelocitiesResult ProcessGoals(float simulationStep, const EntityTransform& currentTransform);

    // will clear all locomotion requests
    void ClearGoals();

    // check whether locomotion is requested
    inline bool HasGoals() const 
    { 
        return !mLocomotionGoals.empty(); 
    }

    // reach orientation
    void OrientToPoint(const glm::vec2& orientToPosition, const EntityTransform& currentTransform);
    void OrientTo(cxx::angle_t orientation);

    // arrive to destination
    void ArriveTo(const glm::vec2& position, bool enableAutoOrientation);

private:
    // check whether desired position / orientation is reached
    bool CheckAtGoal(const LocomotionGoal& locomotionGoal, const EntityTransform& currentTransform) const;

private:
    // params
    float mArriveSpeed = 0.0f;
    float mArrivalTolerance = 0.1f;
    float mOrientSpeed = 0.0f; // rads per second
    float mAlignTolerance = 0.1f;

    std::vector<LocomotionGoal> mLocomotionGoals;
};

//////////////////////////////////////////////////////////////////////////