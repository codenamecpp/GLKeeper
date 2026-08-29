#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureDefs.h"
#include "EntityDefs.h"

//////////////////////////////////////////////////////////////////////////

class CreatureTask final: public cxx::noncopyable
{
    friend class CreatureTaskManager;

public:
    CreatureTask() = default;

    inline CreatureTaskUid GetTaskUid() const { return mTaskUid; }
    inline eCreatureJob GetJobType() const { return mJobType; }
    inline bool HasJobType(eCreatureJob jobType) const
    {
        return (mJobType == jobType);
    }
    inline ePlayerID GetPlayerId() const { return mPlayerId; }
    inline Creature* GetAssignee() const { return mAssignee; }

    // status
    // whether task is no longer valid for whatever reason
    inline bool IsExpired() const { return mIsExpired; }
    inline bool IsOngoing() const { return mIsExpired == false; }

    // target position specifies the world coordinates for the creature to move to and do its work
    //
    // once set it never changes
    bool GetTargetPosition(glm::vec2& targetPosition) const;

    // depending on the task, a target tile might be specified
    //
    // once set it never changes
    bool GetTargetTile(Point2D& targetTile) const;
    bool HasTargetTile() const;

    // depending on the task, a target entities might be specified
    //
    // once set it never changes
    inline EntityHandle GetTargetObject() const { return mTargetObject; }
    inline EntityHandle GetTargetRoom() const { return mTargetRoom; }
    inline EntityHandle GetTargetCreature() const { return mTargetCreature; }

private:
    // configuration
    void Configure(eCreatureJob jobType, CreatureTaskUid taskUid, Creature* assignee);
    void ConfigureTargetObject(EntityHandle entityHandle);
    void ConfigureTargetCreature(EntityHandle entityHandle);
    void ConfigureTargetRoom(EntityHandle entityHandle);
    void ConfigureTargetTile(const Point2D& targetTile);
    void ConfigureTargetPosition(const glm::vec2& targetPosition);

    // pool
    void OnRecycle();

    void SetExpired();

private:
    eCreatureJob mJobType = eCreatureJob_None; 
    ePlayerID mPlayerId = ePlayerID_COUNT;
    CreatureTaskUid mTaskUid {};

    Creature* mAssignee = nullptr;

    EntityHandle mTargetObject {};
    EntityHandle mTargetRoom {};
    EntityHandle mTargetCreature {};

    std::optional<Point2D> mTargetTile {};
    std::optional<glm::vec2> mTargetPosition {};

    bool mIsExpired = false;
};

//////////////////////////////////////////////////////////////////////////