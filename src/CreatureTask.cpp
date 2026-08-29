#include "stdafx.h"
#include "CreatureTask.h"
#include "Creature.h"

void CreatureTask::Configure(eCreatureJob jobType, CreatureTaskUid taskUid, Creature* assignee)
{
    mIsExpired = {};
    mJobType = jobType;
    mTaskUid = taskUid;
    mAssignee = assignee;
    cxx_assert(mAssignee);
    mPlayerId = mAssignee->GetOwnerId();
}

void CreatureTask::OnRecycle()
{
    mJobType = {};
    mPlayerId = ePlayerID_COUNT;
    mTaskUid = {};
    mIsExpired = {};
    mTargetCreature = {};
    mTargetObject = {};
    mTargetRoom = {};
    mAssignee = {};
    mTargetTile.reset();
}

bool CreatureTask::GetTargetPosition(glm::vec2& targetPosition) const
{
    if (mTargetPosition)
    {
        targetPosition = *mTargetPosition;
        return true;
    }
    return false;
}

bool CreatureTask::GetTargetTile(Point2D& targetTile) const
{
    if (mTargetTile)
    {
        targetTile = *mTargetTile;
    }
    return mTargetTile.has_value();
}

bool CreatureTask::HasTargetTile() const
{
    return mTargetTile.has_value();
}

void CreatureTask::ConfigureTargetTile(const Point2D& targetTile)
{
    mTargetTile.emplace(targetTile);
}

void CreatureTask::ConfigureTargetPosition(const glm::vec2& position2d)
{
    mTargetPosition.emplace(position2d);
}

void CreatureTask::SetExpired()
{
    mIsExpired = true;
}

void CreatureTask::ConfigureTargetObject(EntityHandle entityHandle)
{
    mTargetObject = entityHandle;
    cxx_assert(mTargetObject.IsGameObject());
}

void CreatureTask::ConfigureTargetCreature(EntityHandle entityHandle)
{
    mTargetCreature = entityHandle;
    cxx_assert(mTargetCreature.IsCreature());
}

void CreatureTask::ConfigureTargetRoom(EntityHandle entityHandle)
{
    mTargetRoom = entityHandle;
    cxx_assert(mTargetRoom.IsRoom());
}
