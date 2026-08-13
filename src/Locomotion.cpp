#include "stdafx.h"
#include "Locomotion.h"

//////////////////////////////////////////////////////////////////////////

// helpers

inline cxx::angle_t ComputeOrientationTo(const glm::vec2& targetPosition, const EntityTransform& currentTransform)
{
    const glm::vec2 directionToTarget = glm::normalize(targetPosition - currentTransform.GetPosition2d());
    if (glm::length2(directionToTarget) > 0.0f)
    {
        // note:
        // standard orientation (0 rad) is positive X, whereas game entity' normal orientation is negative Y

        return cxx::angle_t::from_radians(::atan2f(directionToTarget.x, directionToTarget.y));
    }

    // fallback to default
    return currentTransform.mOrientation;
}

//////////////////////////////////////////////////////////////////////////

void Locomotion::ResetToDefaults()
{
    SetArrivalTolerance(0.1f);
    SetAlignTolerance(0.1f);
    // init default speed params
    SetArriveSpeed(1.0f);
    SetOrientSpeed(glm::radians(230.0f));
}

void Locomotion::SetArriveSpeed(float moveSpeedPerTile)
{
    cxx_assert(moveSpeedPerTile >= 0.0f);
    mArriveSpeed = MAP_TILE_SIZE * std::max(moveSpeedPerTile, 0.0f);
}

void Locomotion::SetOrientSpeed(float radsPerSecond)
{
    cxx_assert(radsPerSecond >= 0.0f);
    mOrientSpeed = std::max(radsPerSecond, 0.0f);
}

void Locomotion::SetAlignTolerance(float tolerance)
{
    cxx_assert(tolerance >= 0.0f);
    mAlignTolerance = std::max(tolerance, 0.0f);
}

void Locomotion::SetArrivalTolerance(float tolerance)
{
    cxx_assert(tolerance >= 0.0f);
    mArrivalTolerance = std::max(tolerance, 0.0f);
}

Locomotion::VelocitiesResult Locomotion::ProcessGoals(float simulationStep, const EntityTransform& currentTransform)
{
    VelocitiesResult velocitiesResult {};

    for ( ; !mLocomotionGoals.empty(); )
    {
        const LocomotionGoal& currentGoal = mLocomotionGoals.front();

        bool isCurrentGoalReached = true;
        bool isOrienting = currentGoal.mDesiredOrientation || (currentGoal.mAutoOrientation && currentGoal.mDesiredPosition);
        if (isOrienting)
        {
            cxx::angle_t desiredOrientation {};
            if (currentGoal.mAutoOrientation && currentGoal.mDesiredPosition)
            {
                desiredOrientation = ComputeOrientationTo(*currentGoal.mDesiredPosition, currentTransform);
            }
            else
            {
                desiredOrientation = *currentGoal.mDesiredOrientation;
            }

            const float diff = cxx::wrap_angle_to_pi((desiredOrientation - currentTransform.mOrientation).to_radians());
            const float diff_abs = glm::abs(diff);

            if (diff_abs > mAlignTolerance)
            {
                isCurrentGoalReached = false;

                // how much we SHOULD turn this frame
                float turn_step = simulationStep * mOrientSpeed;
                if (turn_step > diff_abs)
                {
                    turn_step = diff_abs;
                }
                velocitiesResult.mAngularVelocity = (turn_step / simulationStep) * glm::sign(diff);
            }
            else 
            {
                // finish
            }
        }

        if (currentGoal.mDesiredPosition)
        {
            const glm::vec2 directionToTarget = (*currentGoal.mDesiredPosition - currentTransform.GetPosition2d());
            const float distanceToTarget = glm::length(directionToTarget);

            if (distanceToTarget > mArrivalTolerance)
            {
                isCurrentGoalReached = false;

                const glm::vec2 unitDirection = directionToTarget / distanceToTarget;
                float distanceToMove = simulationStep * mArriveSpeed;
                if (distanceToMove > distanceToTarget)
                {
                    distanceToMove = distanceToTarget;
                }
                velocitiesResult.mLinearVelocity = unitDirection * distanceToMove / simulationStep;
            }
            else 
            {
                isCurrentGoalReached = true;
                // finish
            }
        }

        if (!isCurrentGoalReached) 
            break;

        mLocomotionGoals.erase(mLocomotionGoals.begin()); 
    }

    return velocitiesResult;
}

void Locomotion::ClearGoals()
{
    mLocomotionGoals.clear();
}

void Locomotion::OrientToPoint(const glm::vec2& orientToPosition, const EntityTransform& currentTransform)
{
    LocomotionGoal& locomotionGoal = mLocomotionGoals.emplace_back();
    locomotionGoal.mDesiredOrientation = ComputeOrientationTo(orientToPosition, currentTransform);
}

void Locomotion::OrientTo(cxx::angle_t orientation)
{
    LocomotionGoal& locomotionGoal = mLocomotionGoals.emplace_back();
    locomotionGoal.mDesiredOrientation = orientation;
}

void Locomotion::ArriveTo(const glm::vec2& position, bool enableAutoOrientation)
{
    LocomotionGoal& locomotionGoal = mLocomotionGoals.emplace_back();
    locomotionGoal.mDesiredPosition = position;
    locomotionGoal.mAutoOrientation = enableAutoOrientation;
}

bool Locomotion::CheckAtGoal(const LocomotionGoal& locomotionGoal, const EntityTransform& currentTransform) const
{
    if (locomotionGoal.mDesiredOrientation)
    {
        const float diff = cxx::wrap_angle_to_pi((*locomotionGoal.mDesiredOrientation - currentTransform.mOrientation).to_radians());
        const float diff_abs = glm::abs(diff);
        // acceptable difference?
        if (diff_abs > mAlignTolerance)
            return false;
    }

    if (locomotionGoal.mDesiredPosition)
    {
        const glm::vec2 directionToTarget = (*locomotionGoal.mDesiredPosition - currentTransform.GetPosition2d());
        const float distanceToTarget = glm::length(directionToTarget);
        // acceptable distance?
        if (distanceToTarget > mArrivalTolerance)
            return false;
    }

    return true;
}
