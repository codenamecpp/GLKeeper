#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureDefs.h"
#include "EntityDefs.h"

//////////////////////////////////////////////////////////////////////////

class CreatureState: public cxx::noncopyable
{
public:
    CreatureState(eCreatureState stateId);
    virtual ~CreatureState();

    inline eCreatureState GetStateId() const { return mStateId; }
    inline bool IsStateId(eCreatureState stateId) const
    {
        return mStateId == stateId;
    }

    void Configure(Creature* creature);

    // pool
    virtual void OnRecycle();

    void EnterState(eCreatureState prevState);
    void LeaveState(eCreatureState nextState);
    void UpdateLogic(float stepDeltaTime);
    void ReceiveMsg(EntityMsg& msgData);

    inline float GetStateDuration() const { return mStateDuration; }

protected:
    inline Creature* GetCreaturePtr() const { return mCreature; }
    inline Creature& GetCreature() const
    {
        cxx_assert(mCreature);
        return *mCreature;
    }

    // starts substate, returns previous action
    CreatureActionPtr StartStateAction(CreatureActionPtr action);

    // accessing current substate
    inline CreatureAction* GetStateAction() const 
    { 
        return mStateAction.get(); 
    }

    //////////////////////////////////////////////////////////////////////////
    // overridables
    virtual void HandleEnterState(eCreatureState prevState) = 0;
    virtual void HandleLeaveState(eCreatureState nextState) = 0;
    virtual void HandleUpdateLogic(float stepDeltaTime) = 0;
    virtual void HandleMessage(EntityMsg& msgData) {}
    //////////////////////////////////////////////////////////////////////////

private:
    eCreatureState mStateId {};
    Creature* mCreature {};

    // sub state
    CreatureActionPtr mStateAction {};

    float mStateDuration {};
};

//////////////////////////////////////////////////////////////////////////